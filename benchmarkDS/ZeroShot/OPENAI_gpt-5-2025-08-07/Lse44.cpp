#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static const size_t MAX_SIZE_BYTES = 1024 * 1024; // 1MB

static std::vector<std::string> splitPath(const std::string& s) {
    std::vector<std::string> parts;
    size_t i = 0;
    while (i < s.size()) {
        size_t j = s.find('/', i);
        if (j == std::string::npos) j = s.size();
        std::string part = s.substr(i, j - i);
        if (part.empty()) throw std::runtime_error("Invalid path component");
        if (part == "." || part == "..") throw std::runtime_error("Path traversal detected");
        parts.push_back(part);
        i = j + 1;
    }
    if (parts.empty()) throw std::runtime_error("Empty path");
    return parts;
}

std::string readSafeFile(const std::string& requested) {
    if (requested.empty()) throw std::runtime_error("Invalid filename");
    if (requested.find('\0') != std::string::npos) throw std::runtime_error("Invalid character in filename");
    if (!requested.empty() && requested[0] == '/') throw std::runtime_error("Absolute paths are not allowed");

    std::vector<std::string> parts = splitPath(requested);

    int dirfd = open("/safe", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        throw std::runtime_error(std::string("Failed to open base dir: ") + std::strerror(errno));
    }

    int cwd_fd = dirfd;
    int fd = -1;
    try {
        // Traverse directories
        for (size_t i = 0; i + 1 < parts.size(); ++i) {
            int next = openat(cwd_fd, parts[i].c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (next < 0) {
                throw std::runtime_error(std::string("Open dir failed: ") + std::strerror(errno));
            }
            if (cwd_fd != dirfd) close(cwd_fd);
            cwd_fd = next;
        }

        // Open final file
        fd = openat(cwd_fd, parts.back().c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        if (fd < 0) {
            throw std::runtime_error(std::string("Open file failed: ") + std::strerror(errno));
        }

        struct stat st {};
        if (fstat(fd, &st) != 0) {
            throw std::runtime_error(std::string("fstat failed: ") + std::strerror(errno));
        }
        if (!S_ISREG(st.st_mode)) {
            throw std::runtime_error("Not a regular file");
        }
        if (st.st_size > (off_t)MAX_SIZE_BYTES) {
            throw std::runtime_error("File too large");
        }

        std::string content;
        content.reserve(st.st_size > 0 ? static_cast<size_t>(st.st_size) : 0);

        char buf[8192];
        size_t remaining = MAX_SIZE_BYTES;
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            if (remaining < static_cast<size_t>(n)) {
                throw std::runtime_error("File grew beyond limit while reading");
            }
            content.append(buf, buf + n);
            remaining -= static_cast<size_t>(n);
        }
        if (n < 0) {
            throw std::runtime_error(std::string("Read failed: ") + std::strerror(errno));
        }

        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        return content;
    } catch (...) {
        if (fd >= 0) close(fd);
        if (cwd_fd != dirfd && cwd_fd >= 0) close(cwd_fd);
        if (dirfd >= 0) close(dirfd);
        throw;
    }
}

int main() {
    std::vector<std::string> tests = {
        "example.txt",
        "../etc/passwd",
        "/etc/passwd",
        "subdir/file.txt",
        "symlink_outside.txt"
    };
    for (const auto& t : tests) {
        try {
            std::string content = readSafeFile(t);
            std::cout << "Read OK: " << t << " (len=" << content.size() << ")\n";
        } catch (const std::exception& e) {
            std::cout << "Read FAIL: " << t << " -> " << e.what() << "\n";
        }
    }
    return 0;
}