#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits>
#include <cstdlib>

namespace SecureFileReader {

static constexpr size_t MAX_READ_SIZE = 1024 * 1024; // 1 MiB cap
static constexpr size_t MAX_COMPONENT_LEN = 255;

static bool isSafeComponent(const std::string &comp) {
    if (comp.empty()) return false;
    if (comp == "." || comp == "..") return false;
    if (comp.size() > MAX_COMPONENT_LEN) return false;
    if (comp.find('/') != std::string::npos) return false;
    return true;
}

static bool splitAndValidate(const std::string &relPath, std::vector<std::string> &components, std::string &err) {
    if (relPath.empty()) {
        err = "Empty path not allowed";
        return false;
    }
    if (!relPath.empty() && relPath[0] == '/') {
        err = "Absolute paths are not allowed";
        return false;
    }
    std::string cur;
    for (size_t i = 0; i < relPath.size(); ++i) {
        char c = relPath[i];
        if (c == '/') {
            if (cur.empty()) {
                err = "Empty path segment detected (e.g., double slashes)";
                return false;
            }
            if (!isSafeComponent(cur)) {
                err = "Unsafe path segment: " + cur;
                return false;
            }
            components.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (cur.empty()) {
        err = "Path cannot end with slash";
        return false;
    }
    if (!isSafeComponent(cur)) {
        err = "Unsafe path segment: " + cur;
        return false;
    }
    components.push_back(cur);
    return true;
}

// Opens a file descriptor to the requested file securely under baseDir.
// Denies symlinks at any path segment and requires a regular file.
static int openFileUnderBase(const std::string &baseDir, const std::string &relPath, std::string &err) {
    if (baseDir.empty()) {
        err = "Base directory must not be empty";
        return -1;
    }

    std::vector<std::string> comps;
    if (!splitAndValidate(relPath, comps, err)) {
        return -1;
    }

    int dirfd = open(baseDir.c_str(), O_RDONLY | O_DIRECTORY);
    if (dirfd < 0) {
        err = "Failed to open base directory: " + std::string(std::strerror(errno));
        return -1;
    }

    // Walk through components without following symlinks
    for (size_t i = 0; i + 1 < comps.size(); ++i) {
        int nextfd = openat(dirfd, comps[i].c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
        if (nextfd < 0) {
            std::string why = std::strerror(errno);
            close(dirfd);
            err = "Failed to open directory segment '" + comps[i] + "': " + why;
            return -1;
        }
        close(dirfd);
        dirfd = nextfd;
    }

    // Open the final component as a file without following symlinks
    int fd = openat(dirfd, comps.back().c_str(), O_RDONLY | O_NOFOLLOW);
    int savedErrno = errno;
    close(dirfd);
    if (fd < 0) {
        err = "Failed to open target file '" + comps.back() + "': " + std::string(std::strerror(savedErrno));
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        std::string why = std::strerror(errno);
        close(fd);
        err = "fstat failed: " + why;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        err = "Path is not a regular file";
        return -1;
    }
    return fd;
}

static bool readAllFromFd(int fd, std::string &out, std::string &err) {
    struct stat st;
    if (fstat(fd, &st) != 0) {
        err = "fstat failed: " + std::string(std::strerror(errno));
        return false;
    }

    if (st.st_size < 0) {
        err = "Invalid file size";
        return false;
    }
    if (static_cast<unsigned long long>(st.st_size) > static_cast<unsigned long long>(MAX_READ_SIZE)) {
        err = "File too large";
        return false;
    }

    size_t remaining = static_cast<size_t>(st.st_size);
    out.clear();
    out.reserve(remaining);

    const size_t BUF_SIZE = 8192;
    std::vector<char> buf(BUF_SIZE);

    while (remaining > 0) {
        size_t toRead = remaining < BUF_SIZE ? remaining : BUF_SIZE;
        ssize_t r = read(fd, buf.data(), toRead);
        if (r < 0) {
            if (errno == EINTR) continue;
            err = "Read error: " + std::string(std::strerror(errno));
            return false;
        }
        if (r == 0) {
            // Unexpected EOF
            err = "Unexpected end of file";
            return false;
        }
        out.append(buf.data(), static_cast<size_t>(r));
        remaining -= static_cast<size_t>(r);
    }
    return true;
}

// Public API: read file content securely under a base directory
static bool readFileUnderBase(const std::string &baseDir, const std::string &relPath, std::string &content, std::string &err) {
    int fd = openFileUnderBase(baseDir, relPath, err);
    if (fd < 0) return false;
    bool ok = readAllFromFd(fd, content, err);
    int saved = errno;
    close(fd);
    errno = saved;
    return ok;
}

} // namespace SecureFileReader

// Test helpers
static bool writeFile(const std::string &path, const std::string &data) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return false;
    const char *p = data.data();
    size_t left = data.size();
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        p += static_cast<size_t>(w);
        left -= static_cast<size_t>(w);
    }
    int rc = close(fd);
    return rc == 0;
}

int main(int argc, char *argv[]) {
    using namespace SecureFileReader;

    if (argc >= 3) {
        std::string base = argv[1];
        std::string rel = argv[2];
        std::string content, err;
        if (readFileUnderBase(base, rel, content, err)) {
            // Print to stdout
            std::cout.write(content.data(), static_cast<std::streamsize>(content.size()));
            return 0;
        } else {
            std::cerr << "Error: " << err << std::endl;
            return 1;
        }
    }

    // Self-tests (5 test cases)
    char tmpl[] = "/tmp/task70_cpp.XXXXXX";
    char *dir = mkdtemp(tmpl);
    if (!dir) {
        std::cerr << "Failed to create temp dir" << std::endl;
        return 1;
    }
    std::string baseDir = dir;

    // Prepare files and dirs
    std::string fileA = baseDir + "/a.txt";
    std::string subdir = baseDir + "/subdir";
    std::string fileB = subdir + "/b.txt";
    std::string symlinkA = baseDir + "/link_to_a";

    if (mkdir(subdir.c_str(), 0700) != 0) {
        std::cerr << "mkdir failed" << std::endl;
        return 1;
    }
    if (!writeFile(fileA, "Hello Secure World\n")) {
        std::cerr << "write a.txt failed" << std::endl;
        return 1;
    }
    if (!writeFile(fileB, "Inside subdir\n")) {
        std::cerr << "write b.txt failed" << std::endl;
        return 1;
    }
    // Create a symlink (may fail on some systems; ignore errors if not supported)
    symlink("a.txt", symlinkA.c_str());

    // Test 1: valid read
    {
        std::string content, err;
        bool ok = readFileUnderBase(baseDir, "a.txt", content, err);
        std::cout << "TEST1 valid: " << (ok ? "OK" : "FAIL") << "\n";
        if (ok) std::cout << content;
        else std::cout << "ERR: " << err << "\n";
    }
    // Test 2: absolute path should fail
    {
        std::string content, err;
        bool ok = readFileUnderBase(baseDir, "/etc/passwd", content, err);
        std::cout << "TEST2 absolute: " << (ok ? "UNEXPECTED_OK" : "EXPECTED_FAIL") << "\n";
        if (!ok) std::cout << "ERR: " << err << "\n";
    }
    // Test 3: traversal should fail
    {
        std::string content, err;
        bool ok = readFileUnderBase(baseDir, "../a.txt", content, err);
        std::cout << "TEST3 traversal: " << (ok ? "UNEXPECTED_OK" : "EXPECTED_FAIL") << "\n";
        if (!ok) std::cout << "ERR: " << err << "\n";
    }
    // Test 4: symlink should fail
    {
        std::string content, err;
        bool ok = readFileUnderBase(baseDir, "link_to_a", content, err);
        std::cout << "TEST4 symlink: " << (ok ? "UNEXPECTED_OK" : "EXPECTED_FAIL") << "\n";
        if (!ok) std::cout << "ERR: " << err << "\n";
    }
    // Test 5: directory should fail
    {
        std::string content, err;
        bool ok = readFileUnderBase(baseDir, "subdir", content, err);
        std::cout << "TEST5 directory: " << (ok ? "UNEXPECTED_OK" : "EXPECTED_FAIL") << "\n";
        if (!ok) std::cout << "ERR: " << err << "\n";
    }

    return 0;
}