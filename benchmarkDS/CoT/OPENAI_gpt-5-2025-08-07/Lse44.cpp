// Step 1: Read a requested file from /safe directory securely
// Step 2: Security: prevent path traversal, enforce base boundary, avoid symlink escape, limit size, O_NOFOLLOW
// Step 3/5: Secure coding and output

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cerrno>
#include <climits>
#include <cstring>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

static const size_t MAX_BYTES = 5 * 1024 * 1024; // 5 MiB

static std::string sanitizeRelative(const std::string& in) {
    std::string s = in;
    for (char& c : s) {
        if (c == '\\') c = '/';
    }
    while (!s.empty() && s.front() == '/') {
        s.erase(s.begin());
    }
    return s;
}

std::string readSafeFile(const std::string& requestedPath) {
    // Resolve base real path
    char baseReal[PATH_MAX];
    if (!realpath("/safe", baseReal)) {
        throw std::runtime_error(std::string("Failed to resolve base: ") + strerror(errno));
    }

    // Sanitize and combine
    std::string rel = sanitizeRelative(requestedPath);
    std::string combined = std::string(baseReal) + "/" + rel;

    // Canonicalize target
    char targetReal[PATH_MAX];
    if (!realpath(combined.c_str(), targetReal)) {
        throw std::runtime_error(std::string("Failed to resolve target: ") + strerror(errno));
    }

    // Ensure target remains within base (directory boundary aware)
    size_t baseLen = std::strlen(baseReal);
    if (std::strncmp(targetReal, baseReal, baseLen) != 0 ||
        !(targetReal[baseLen] == '/' || targetReal[baseLen] == '\0')) {
        throw std::runtime_error("Access denied: target escapes base directory");
    }

    // Ensure regular file and size limit using lstat
    struct stat st;
    if (lstat(targetReal, &st) != 0) {
        throw std::runtime_error(std::string("lstat failed: ") + strerror(errno));
    }
    if (!S_ISREG(st.st_mode)) {
        throw std::runtime_error("Not a regular file");
    }
    if ((size_t)st.st_size > MAX_BYTES) {
        throw std::runtime_error("File too large");
    }

    // Open without following symlinks
    int fd = open(targetReal, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        throw std::runtime_error(std::string("open failed: ") + strerror(errno));
    }

    // Re-check after open
    struct stat st2;
    if (fstat(fd, &st2) != 0) {
        int e = errno;
        close(fd);
        throw std::runtime_error(std::string("fstat failed: ") + strerror(e));
    }
    if (!S_ISREG(st2.st_mode)) {
        close(fd);
        throw std::runtime_error("Opened target is not a regular file");
    }
    if ((size_t)st2.st_size > MAX_BYTES) {
        close(fd);
        throw std::runtime_error("File too large");
    }

    std::string data;
    data.resize((size_t)st2.st_size);
    size_t total = 0;
    while (total < data.size()) {
        ssize_t r = read(fd, &data[total], data.size() - total);
        if (r < 0) {
            int e = errno;
            close(fd);
            throw std::runtime_error(std::string("read failed: ") + strerror(e));
        }
        if (r == 0) break;
        total += (size_t)r;
    }
    close(fd);
    data.resize(total);
    return data;
}

int main() {
    // Step 4: Code review done, testing with 5 cases
    std::vector<std::string> tests = {
        "example.txt",
        "subdir/data.txt",
        "../etc/passwd",
        "/safe/../safe/secret.txt",
        "nonexistent.txt"
    };
    for (const auto& t : tests) {
        std::cout << "=== Test: " << t << " ===\n";
        try {
            std::string content = readSafeFile(t);
            std::cout << "Read OK. Length: " << content.size() << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }
    return 0;
}