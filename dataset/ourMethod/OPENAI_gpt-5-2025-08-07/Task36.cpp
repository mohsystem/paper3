#include <iostream>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <cstdlib>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

static bool isValidFilename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    for (char c : name) {
        if (!( (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '.' || c == '_' || c == '-' )) {
            return false;
        }
    }
    return true;
}

static bool canonicalizeDir(const std::string& dir, std::string& outCanonical, std::string& err) {
    if (dir.empty() || dir.size() > PATH_MAX - 1) {
        err = "Invalid base directory length";
        return false;
    }
    char resolved[PATH_MAX];
    errno = 0;
    if (realpath(dir.c_str(), resolved) == nullptr) {
        err = std::string("Failed to resolve base directory: ") + std::strerror(errno);
        return false;
    }
    struct stat st{};
    if (stat(resolved, &st) != 0) {
        err = std::string("Failed to stat base directory: ") + std::strerror(errno);
        return false;
    }
    if (!S_ISDIR(st.st_mode)) {
        err = "Base path is not a directory";
        return false;
    }
    outCanonical.assign(resolved);
    return true;
}

static bool readAllFromFd(int fd, std::string& outContent, std::string& err) {
    outContent.clear();
    std::vector<char> buf;
    buf.reserve(8192);
    size_t total = 0;
    while (true) {
        char chunk[8192];
        ssize_t n = ::read(fd, chunk, sizeof(chunk));
        if (n < 0) {
            if (errno == EINTR) continue;
            err = std::string("Read error: ") + std::strerror(errno);
            return false;
        }
        if (n == 0) break; // EOF
        if (total + static_cast<size_t>(n) > MAX_FILE_SIZE) {
            err = "File too large (exceeds limit)";
            return false;
        }
        buf.insert(buf.end(), chunk, chunk + n);
        total += static_cast<size_t>(n);
    }
    outContent.assign(buf.begin(), buf.end());
    return true;
}

// Reads a file within baseDir with a strict filename (no path separators).
// Returns true on success and sets outContent; else returns false and sets errMsg.
static bool secureReadFile(const std::string& baseDir,
                           const std::string& userFilename,
                           std::string& outContent,
                           std::string& errMsg) {
    outContent.clear();
    errMsg.clear();

    if (!isValidFilename(userFilename)) {
        errMsg = "Invalid filename: only letters, digits, '.', '_', '-' allowed; length 1..255";
        return false;
    }

    std::string canonBase;
    if (!canonicalizeDir(baseDir, canonBase, errMsg)) {
        return false;
    }

    // Build absolute path: canonBase + "/" + userFilename
    std::string fullPath = canonBase;
    if (!fullPath.empty() && fullPath.back() != '/') fullPath.push_back('/');
    fullPath += userFilename;

    int fd = ::open(fullPath.c_str(), O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        errMsg = std::string("Open failed: ") + std::strerror(errno);
        return false;
    }

    struct stat st{};
    if (fstat(fd, &st) != 0) {
        errMsg = std::string("fstat failed: ") + std::strerror(errno);
        ::close(fd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        errMsg = "Not a regular file";
        ::close(fd);
        return false;
    }
    if (st.st_size > 0 && static_cast<size_t>(st.st_size) > MAX_FILE_SIZE) {
        errMsg = "File too large (exceeds limit)";
        ::close(fd);
        return false;
    }

    bool ok = readAllFromFd(fd, outContent, errMsg);
    ::close(fd);
    return ok;
}

// Helper functions for tests
static std::string pathJoin(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

static bool makeDir(const std::string& dir) {
    if (::mkdir(dir.c_str(), 0700) == 0) return true;
    if (errno == EEXIST) return true;
    return false;
}

static bool writeFileAtomic(const std::string& path, const std::string& content, std::string& err) {
    // Create file exclusively; refuse if exists
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        err = std::string("Create failed: ") + std::strerror(errno);
        return false;
    }
    size_t total = 0;
    while (total < content.size()) {
        ssize_t n = ::write(fd, content.data() + total, content.size() - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            err = std::string("Write failed: ") + std::strerror(errno);
            ::close(fd);
            return false;
        }
        total += static_cast<size_t>(n);
    }
    if (::fsync(fd) != 0) {
        err = std::string("fsync failed: ") + std::strerror(errno);
        ::close(fd);
        return false;
    }
    if (::close(fd) != 0) {
        err = std::string("close failed: ") + std::strerror(errno);
        return false;
    }
    return true;
}

static std::string makeTempBaseDir() {
    char tmpl[] = "/tmp/secure_read_XXXXXX";
    char* res = ::mkdtemp(tmpl);
    if (res == nullptr) {
        return std::string();
    }
    return std::string(res);
}

int main() {
    // Prepare a secure base directory and test files
    std::string base = makeTempBaseDir();
    if (base.empty()) {
        std::cerr << "Failed to create temp base directory\n";
        return 1;
    }
    // Create files/directories for tests
    std::string err;
    // file1.txt
    writeFileAtomic(pathJoin(base, "file1.txt"), "Hello from file1\nLine2\n", err);
    // Directory "adir"
    makeDir(pathJoin(base, "adir"));
    // Subdir and nested file (will be blocked by filename policy)
    makeDir(pathJoin(base, "subdir"));
    writeFileAtomic(pathJoin(base, "subdir_nested.txt"), "This is not in subdir, just named differently\n", err);
    // Symlink to file1 (may fail on some systems; ignore result)
    ::symlink(pathJoin(base, "file1.txt").c_str(), pathJoin(base, "link_to_file1").c_str());

    // Five test cases
    const std::pair<std::string, std::string> tests[] = {
        { "file1.txt", "Valid existing file" },
        { "nonexistent.txt", "Non-existent file" },
        { "link_to_file1", "Symlink should be refused" },
        { "adir", "Directory should be refused" },
        { "../etc_passwd", "Path traversal attempt (invalid filename)" }
    };

    for (const auto& t : tests) {
        std::string content, e;
        bool ok = secureReadFile(base, t.first, content, e);
        std::cout << "Test: " << t.second << " [" << t.first << "]\n";
        if (ok) {
            std::cout << "----- BEGIN CONTENT -----\n";
            std::cout << content;
            if (!content.empty() && content.back() != '\n') std::cout << "\n";
            std::cout << "------ END CONTENT ------\n";
        } else {
            std::cout << "Error: " << e << "\n";
        }
        std::cout << "=========================\n";
    }

    // Example of reading a user-specified filename (simulated here)
    // In a real environment, obtain user input safely and pass to secureReadFile.
    return 0;
}