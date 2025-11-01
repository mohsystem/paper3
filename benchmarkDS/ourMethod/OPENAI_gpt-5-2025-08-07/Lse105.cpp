#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <chrono>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

static bool validate_filename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char c : name) {
        if (c == '/' || c == '\\') return false;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static std::string random_hex(size_t bytes) {
    std::vector<unsigned char> buf(bytes, 0);
    // Use /dev/urandom
    int ur = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (ur >= 0) {
        size_t off = 0;
        while (off < bytes) {
            ssize_t n = read(ur, buf.data() + off, bytes - off);
            if (n <= 0) break;
            off += static_cast<size_t>(n);
        }
        close(ur);
        if (buf.size() == bytes) {
            std::ostringstream oss;
            static const char* hexd = "0123456789abcdef";
            for (unsigned char b : buf) {
                oss << hexd[(b >> 4) & 0xF] << hexd[b & 0xF];
            }
            return oss.str();
        }
    }
    // Fallback to PRNG if /dev/urandom fails (not ideal, but ensures function continues)
    std::mt19937_64 rng(static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream oss;
    static const char* hexd = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        unsigned char b = static_cast<unsigned char>(dist(rng));
        oss << hexd[(b >> 4) & 0xF] << hexd[b & 0xF];
    }
    return oss.str();
}

static int open_dirfd_secure(const std::string& base_dir) {
    int flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY | O_NOFOLLOW;
    int dirfd = open(base_dir.c_str(), flags);
    if (dirfd < 0) return -1;
    struct stat st;
    if (fstat(dirfd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(dirfd);
        errno = ENOTDIR;
        return -1;
    }
    return dirfd;
}

static bool write_all(int fd, const void* buf, size_t len) {
    const unsigned char* p = static_cast<const unsigned char*>(buf);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, p + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += static_cast<size_t>(n);
    }
    return true;
}

bool create_secret_file(const std::string& base_dir, const std::string& filename, const std::string& content) {
    if (!validate_filename(filename)) {
        return false;
    }
    int dirfd = open_dirfd_secure(base_dir);
    if (dirfd < 0) {
        return false;
    }

    std::string tmpname = ".tmp." + random_hex(16);

    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW;
    mode_t mode = 0600;
    int fd = openat(dirfd, tmpname.c_str(), flags, mode);
    if (fd < 0) {
        close(dirfd);
        return false;
    }

    bool ok = write_all(fd, content.data(), content.size());
    if (ok) {
        if (fsync(fd) != 0) ok = false;
    }
    if (ok) {
        // Set restrictive permissions explicitly (already 0600 at creation)
        if (fchmod(fd, 0600) != 0) ok = false;
    }

    if (ok) {
        if (renameat(dirfd, tmpname.c_str(), dirfd, filename.c_str()) != 0) {
            ok = false;
        }
    }

    // Persist directory metadata (rename)
    if (ok) {
        if (fsync(dirfd) != 0) ok = false;
    }

    close(fd);
    close(dirfd);
    return ok;
}

int main() {
    const std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    bool all_ok = true;
    for (int i = 1; i <= 5; ++i) {
        char templ[] = "/tmp/secure_secret_test_cpp_XXXXXX";
        char* dir = mkdtemp(templ);
        if (!dir) {
            std::cout << "Test " << i << " FAILED: mkdtemp\n";
            all_ok = false;
            continue;
        }
        bool ok = create_secret_file(dir, "secret.txt", content);
        std::cout << "Test " << i << (ok ? " OK: " : " FAILED: ") << dir << "/secret.txt" << "\n";
        if (!ok) all_ok = false;
    }
    return all_ok ? 0 : 1;
}