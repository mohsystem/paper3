#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <spawn.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

extern char **environ;

struct CatResult {
    bool ok;
    std::string output;
    std::string error;
};

static bool isValidComponent(const std::string &comp) {
    if (comp.empty() || comp == "." || comp == "..") return false;
    // Disallow NUL and control chars
    for (unsigned char ch : comp) {
        if (ch == 0 || ch < 0x20) return false;
    }
    return true;
}

static bool isValidUserPath(const std::string &p) {
    if (p.empty() || p.size() > 4096) return false;
    if (p[0] == '/') return false; // must be relative
    // Normalize and validate components
    size_t start = 0;
    bool hadComponent = false;
    while (start <= p.size()) {
        size_t slash = p.find('/', start);
        std::string comp = (slash == std::string::npos) ? p.substr(start) : p.substr(start, slash - start);
        if (!comp.empty()) {
            if (!isValidComponent(comp)) return false;
            hadComponent = true;
        }
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    return hadComponent;
}

static bool pickCatPath(std::string &catPath, std::string &err) {
    const char* candidates[] = {"/bin/cat", "/usr/bin/cat"};
    for (const char* c : candidates) {
        if (access(c, X_OK) == 0) {
            catPath = c;
            return true;
        }
    }
    err = "cat executable not found at /bin/cat or /usr/bin/cat";
    return false;
}

static bool openUnderBase(const std::string &baseDir, const std::string &userPath, int &outFd, std::string &err) {
    outFd = -1;
    if (!isValidUserPath(userPath)) {
        err = "Invalid user path";
        return false;
    }

    char baseReal[PATH_MAX];
    if (!realpath(baseDir.c_str(), baseReal)) {
        err = std::string("Failed to resolve base dir: ") + strerror(errno);
        return false;
    }
    int baseFd = open(baseReal, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (baseFd < 0) {
        err = std::string("Failed to open base dir: ") + strerror(errno);
        return false;
    }

    // Walk components to prevent symlinks in directories
    int dirFd = baseFd;
    baseFd = -1; // transferred ownership
    size_t start = 0;
    std::vector<std::string> comps;
    while (start <= userPath.size()) {
        size_t slash = userPath.find('/', start);
        std::string comp = (slash == std::string::npos) ? userPath.substr(start) : userPath.substr(start, slash - start);
        if (!comp.empty()) comps.push_back(comp);
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    if (comps.empty()) {
        close(dirFd);
        err = "Empty path after normalization";
        return false;
    }

    // Open intermediate directories (if any) with O_NOFOLLOW
    for (size_t i = 0; i + 1 < comps.size(); ++i) {
        int nextFd = openat(dirFd, comps[i].c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (nextFd < 0) {
            err = std::string("Failed to open directory '") + comps[i] + "': " + strerror(errno);
            close(dirFd);
            return false;
        }
        close(dirFd);
        dirFd = nextFd;
    }

    // Open final file (no symlinks)
    int fileFd = openat(dirFd, comps.back().c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fileFd < 0) {
        err = std::string("Failed to open file: ") + strerror(errno);
        close(dirFd);
        return false;
    }
    struct stat st{};
    if (fstat(fileFd, &st) != 0) {
        err = std::string("fstat failed: ") + strerror(errno);
        close(fileFd);
        close(dirFd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        err = "Target is not a regular file";
        close(fileFd);
        close(dirFd);
        return false;
    }
    close(dirFd);
    outFd = fileFd;
    return true;
}

CatResult display_file_via_command(const std::string &baseDir, const std::string &userPath) {
    CatResult res{false, "", ""};
    int fileFd = -1;
    if (!openUnderBase(baseDir, userPath, fileFd, res.error)) {
        return res;
    }

    std::string catPath;
    if (!pickCatPath(catPath, res.error)) {
        close(fileFd);
        return res;
    }

    int pipefd[2];
#if defined(O_CLOEXEC)
    if (pipe2(pipefd, O_CLOEXEC) != 0) {
#else
    if (pipe(pipefd) != 0) {
#endif
        res.error = std::string("pipe failed: ") + strerror(errno);
        close(fileFd);
        return res;
    }
#ifndef O_CLOEXEC
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);
#endif

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        res.error = "posix_spawn_file_actions_init failed";
        close(fileFd);
        close(pipefd[0]); close(pipefd[1]);
        return res;
    }

    // Child: stdin <- fileFd, stdout -> pipefd[1]
    posix_spawn_file_actions_adddup2(&actions, fileFd, STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);
    posix_spawn_file_actions_addclose(&actions, fileFd);

    const char* argv[] = {"cat", "-", nullptr};
    pid_t pid = -1;
    int sp = posix_spawn(&pid, catPath.c_str(), &actions, nullptr, const_cast<char* const*>(argv), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(fileFd);
    close(pipefd[1]); // parent closes write end
    if (sp != 0) {
        res.error = std::string("posix_spawn failed: ") + strerror(sp);
        close(pipefd[0]);
        return res;
    }

    // Read from pipe
    constexpr size_t MAX_OUTPUT = 10 * 1024 * 1024; // 10 MB cap
    std::string out;
    out.reserve(4096);
    char buf[8192];
    ssize_t r;
    size_t total = 0;
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (total + static_cast<size_t>(r) > MAX_OUTPUT) {
            res.error = "Output exceeds maximum allowed size";
            close(pipefd[0]);
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            return res;
        }
        out.append(buf, buf + r);
        total += static_cast<size_t>(r);
    }
    int savedErrno = errno;
    close(pipefd[0]);
    if (r < 0) {
        res.error = std::string("read failed: ") + strerror(savedErrno);
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        return res;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        res.error = std::string("waitpid failed: ") + strerror(errno);
        return res;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        res.error = "cat process failed";
        return res;
    }

    res.ok = true;
    res.output = std::move(out);
    return res;
}

// Helper to write file safely
static bool writeFile(const std::string &path, const std::string &data, std::string &err) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) { err = std::string("open for write failed: ") + strerror(errno); return false; }
    ssize_t off = 0;
    while (off < (ssize_t)data.size()) {
        ssize_t w = write(fd, data.data() + off, data.size() - off);
        if (w < 0) { err = std::string("write failed: ") + strerror(errno); close(fd); return false; }
        off += w;
    }
    if (fsync(fd) != 0) { err = std::string("fsync failed: ") + strerror(errno); close(fd); return false; }
    if (close(fd) != 0) { err = std::string("close failed: ") + strerror(errno); return false; }
    return true;
}

int main() {
    // Prepare a temporary base directory with test files
    char tmpl[] = "./safe_cat_cppXXXXXX";
    char* dir = mkdtemp(tmpl);
    if (!dir) {
        std::cerr << "Failed to create temp dir: " << strerror(errno) << "\n";
        return 1;
    }
    std::string base = dir;
    std::string err;

    // Create subdir
    std::string subdir = base + "/sub";
    if (mkdir(subdir.c_str(), 0700) != 0) {
        std::cerr << "Failed to create subdir: " << strerror(errno) << "\n";
        return 1;
    }

    // Create files
    writeFile(base + "/a.txt", "Hello A\n", err);
    writeFile(base + "/b.txt", "Line1\nLine2\n", err);
    writeFile(subdir + "/c.txt", "Subdir content\n", err);
    writeFile(base + "/d with spaces.txt", "Spaces in name\n", err);
    // Binary-like content
    std::string bin = std::string("Bin\0Data\n", 9);
    writeFile(base + "/e.bin", bin, err);

    // 5 test cases
    std::vector<std::string> tests = {
        "a.txt",
        "b.txt",
        "sub/c.txt",
        "d with spaces.txt",
        "e.bin"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        CatResult r = display_file_via_command(base, tests[i]);
        std::cout << "Test " << (i+1) << " (" << tests[i] << "): ";
        if (r.ok) {
            std::cout << "[OK]\n";
            std::cout << "Content (" << r.output.size() << " bytes):\n";
            std::cout.write(r.output.data(), r.output.size());
            std::cout << "\n";
        } else {
            std::cout << "[ERR] " << r.error << "\n";
        }
    }

    // Negative test (not counted among 5): attempt traversal
    CatResult neg = display_file_via_command(base, "../should_not_work.txt");
    std::cout << "Negative traversal test: " << (neg.ok ? "unexpected OK" : "blocked: " + neg.error) << "\n";

    return 0;
}