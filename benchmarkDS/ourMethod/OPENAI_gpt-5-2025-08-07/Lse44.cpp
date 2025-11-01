#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cctype>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

static const char* BASE_DIR = "/safe";
static const size_t MAX_SIZE = 4 * 1024 * 1024; // 4 MiB

static bool is_valid_name(const std::string& s) {
    if (s.empty() || s.size() > 255) return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

std::vector<unsigned char> readSafeFile(const std::string& name, bool& ok) {
    ok = false;
    std::vector<unsigned char> out;

    if (!is_valid_name(name)) {
        return out;
    }

    int dirfd = open(BASE_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        return out;
    }

    int fd = openat(dirfd, name.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        close(dirfd);
        return out;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        close(dirfd);
        return out;
    }

    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dirfd);
        return out;
    }

    if (st.st_size < 0 || static_cast<unsigned long long>(st.st_size) > MAX_SIZE) {
        close(fd);
        close(dirfd);
        return out;
    }

    size_t to_read = static_cast<size_t>(st.st_size);
    out.resize(to_read);
    size_t off = 0;
    while (off < to_read) {
        ssize_t r = read(fd, out.data() + off, to_read - off);
        if (r < 0) {
            out.clear();
            close(fd);
            close(dirfd);
            return out;
        }
        if (r == 0) break;
        off += static_cast<size_t>(r);
    }
    out.resize(off);
    ok = true;
    close(fd);
    close(dirfd);
    return out;
}

static std::string sha1_hex(const std::vector<unsigned char>& data) {
    // Minimal SHA-1 using OpenSSL would add dependency; instead, simple dummy hash for output stability.
    unsigned int h = 2166136261u;
    for (unsigned char b : data) {
        h ^= b;
        h *= 16777619u;
    }
    char buf[9];
    snprintf(buf, sizeof(buf), "%08x", h);
    return std::string(buf);
}

int main() {
    const char* tests[5] = {
        "example.txt",
        "does_not_exist.txt",
        "../passwd",
        "valid-file_123",
        "bad\\name"
    };
    for (const char* t : tests) {
        bool ok = false;
        std::vector<unsigned char> data = readSafeFile(std::string(t), ok);
        if (ok) {
            std::cout << "OK " << t << " size=" << data.size() << " hash=" << sha1_hex(data) << "\n";
        } else {
            std::cout << "ERR " << t << "\n";
        }
    }
    return 0;
}