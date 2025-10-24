#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cctype>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/file.h>

struct ReadResult {
    bool ok;
    std::string data;
    std::string error;
};

static bool isAllowedPathChar(char c) {
    return (std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-' || c=='.' || c=='/');
}

static std::vector<std::string> splitPath(const std::string& p) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : p) {
        if (c == '/') {
            if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            } else {
                // skip empty segments caused by consecutive slashes
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    return parts;
}

class FD {
public:
    int fd;
    FD(): fd(-1) {}
    explicit FD(int f): fd(f) {}
    ~FD() { if (fd >= 0) ::close(fd); }
    FD(const FD&) = delete;
    FD& operator=(const FD&) = delete;
    FD(FD&& other) noexcept : fd(other.fd) { other.fd = -1; }
    FD& operator=(FD&& other) noexcept {
        if (this != &other) {
            if (fd >= 0) ::close(fd);
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int get() const { return fd; }
    int release() { int t = fd; fd = -1; return t; }
};

static ReadResult readFileWithinBase(const std::string& baseDir, const std::string& userPath, size_t maxBytes = 1024 * 1024) {
    ReadResult rr{false, "", ""};

    if (baseDir.empty()) {
        rr.error = "Base directory is empty";
        return rr;
    }

    if (userPath.empty()) {
        rr.error = "Filename is empty";
        return rr;
    }

    if (userPath.size() > 1024) {
        rr.error = "Filename too long";
        return rr;
    }

    if (userPath[0] == '/') {
        rr.error = "Absolute paths are not allowed";
        return rr;
    }

    for (char c : userPath) {
        if (c == '\0') {
            rr.error = "Invalid NUL character in path";
            return rr;
        }
        if (!isAllowedPathChar(c)) {
            rr.error = "Invalid character in path";
            return rr;
        }
    }

    std::vector<std::string> parts = splitPath(userPath);
    if (parts.empty()) {
        rr.error = "Invalid path";
        return rr;
    }
    for (const auto& s : parts) {
        if (s == "..") {
            rr.error = "Parent directory reference '..' is not allowed";
            return rr;
        }
        if (s.empty()) {
            rr.error = "Empty path segment";
            return rr;
        }
        if (s.size() > 255) {
            rr.error = "Path segment too long";
            return rr;
        }
    }

    // Open base directory securely
    FD baseFd(::open(baseDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (baseFd.get() < 0) {
        rr.error = std::string("Failed to open base directory: ") + std::strerror(errno);
        return rr;
    }

    // Traverse directories without following symlinks
    FD dirFd(baseFd.release());
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        FD nextFd(::openat(dirFd.get(), parts[i].c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        if (nextFd.get() < 0) {
            rr.error = std::string("Failed to access directory '") + parts[i] + "': " + std::strerror(errno);
            return rr;
        }
        dirFd = std::move(nextFd);
    }

    // Open final component as a file without following symlinks
    FD fileFd(::openat(dirFd.get(), parts.back().c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    if (fileFd.get() < 0) {
        rr.error = std::string("Failed to open file: ") + std::strerror(errno);
        return rr;
    }

    struct stat st;
    if (fstat(fileFd.get(), &st) != 0) {
        rr.error = std::string("Failed to stat file: ") + std::strerror(errno);
        return rr;
    }
    if (!S_ISREG(st.st_mode)) {
        rr.error = "Not a regular file";
        return rr;
    }

    std::string out;
    out.reserve((st.st_size > 0 && static_cast<size_t>(st.st_size) < maxBytes) ? static_cast<size_t>(st.st_size) : 0);

    const size_t BUF = 8192;
    std::vector<char> buf(BUF);
    size_t total = 0;
    while (true) {
        ssize_t r = ::read(fileFd.get(), buf.data(), buf.size());
        if (r < 0) {
            if (errno == EINTR) continue;
            rr.error = std::string("Read error: ") + std::strerror(errno);
            return rr;
        }
        if (r == 0) break;
        if (total + static_cast<size_t>(r) > maxBytes) {
            rr.error = "File too large";
            return rr;
        }
        out.append(buf.data(), static_cast<size_t>(r));
        total += static_cast<size_t>(r);
    }

    rr.ok = true;
    rr.data = std::move(out);
    return rr;
}

static bool ensureDir(const std::string& path) {
    if (::mkdir(path.c_str(), 0700) == 0) return true;
    if (errno == EEXIST) {
        struct stat st;
        if (::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
    }
    return false;
}

static bool writeFileAtomic(const std::string& dir, const std::string& name, const std::string& content) {
    // Create temp file then rename
    std::string tmpName = name + ".tmpXXXXXX";
    std::vector<char> tmpl(tmpName.begin(), tmpName.end());
    tmpl.push_back('\0');

    FD dirFd(::open(dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (dirFd.get() < 0) return false;

    // Create temp in dir using openat and mkstemp-like pattern
    // Since mkstemp doesn't support openat directly, construct full path safely
    std::string fullTmp = dir + "/" + std::string(tmpl.data());
    std::vector<char> fullTmpl(fullTmp.begin(), fullTmp.end());
    fullTmpl.push_back('\0');
    int tfd = ::mkstemp(fullTmpl.data());
    if (tfd < 0) return false;
    FD tmpFd(tfd);

    ssize_t written = 0;
    const char* p = content.data();
    size_t left = content.size();
    while (left > 0) {
        ssize_t w = ::write(tmpFd.get(), p, left > 8192 ? 8192 : left);
        if (w < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p += w;
        left -= static_cast<size_t>(w);
        written += w;
    }
    if (::fsync(tmpFd.get()) != 0) return false;

    // Final target
    std::string finalPath = dir + "/" + name;

    // Use rename to atomically move
    if (::rename(fullTmpl.data(), finalPath.c_str()) != 0) {
        // Cleanup tmp
        ::unlink(fullTmpl.data());
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Prepare sandbox and test files
    const std::string base = "sandbox_cpp";
    const std::string sub = base + "/sub";
    if (!ensureDir(base)) {
        std::cerr << "Failed to create base sandbox directory\n";
        return 1;
    }
    if (!ensureDir(sub)) {
        std::cerr << "Failed to create sub directory\n";
        return 1;
    }
    // Create files securely
    (void)writeFileAtomic(base, "a.txt", "Hello from A\n");
    (void)writeFileAtomic(sub, "b.txt", "Hello from B in subdir\n");

    // Define 5 tests. Test 1 uses user argument if provided, otherwise "a.txt".
    std::vector<std::string> tests;
    if (argc > 1) {
        tests.push_back(argv[1]);
    } else {
        tests.push_back("a.txt");
    }
    tests.push_back("sub/b.txt");        // valid nested file
    tests.push_back("../etc/passwd");    // path traversal attempt
    tests.push_back("missing.txt");      // non-existent
    tests.push_back("sub");              // a directory, not a regular file

    for (size_t i = 0; i < tests.size(); ++i) {
        ReadResult r = readFileWithinBase(base, tests[i]);
        std::cout << "Test " << (i+1) << " [" << tests[i] << "]: ";
        if (r.ok) {
            std::cout << "SUCCESS, bytes=" << r.data.size() << "\n";
            // Print first line or up to 60 chars for brevity
            std::string preview = r.data.substr(0, 60);
            for (char &c : preview) { if (c == '\n') c = ' '; }
            std::cout << "  Preview: " << preview << (r.data.size() > 60 ? "..." : "") << "\n";
        } else {
            std::cout << "ERROR: " << r.error << "\n";
        }
    }
    return 0;
}