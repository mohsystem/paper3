#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __linux__
#include <sys/random.h>
#endif

static bool fill_random_bytes(unsigned char* buf, size_t len) {
#ifdef __linux__
    ssize_t r = getrandom(buf, len, 0);
    if (r == (ssize_t)len) return true;
#endif
    int ur = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (ur < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t got = read(ur, buf + off, len - off);
        if (got <= 0) { close(ur); return false; }
        off += (size_t)got;
    }
    close(ur);
    return true;
}

static std::string hex_encode(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        unsigned char b = data[i];
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

bool write_important_config(const std::string& base_dir) {
    if (base_dir.empty() || base_dir.size() > 4096) return false;

    int dirfd = open(base_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return false;

    struct stat ds;
    if (fstat(dirfd, &ds) != 0 || !S_ISDIR(ds.st_mode)) {
        close(dirfd);
        return false;
    }

    std::string tmpname;
    int fd = -1;
    const mode_t mode600 = 0600;
    for (int i = 0; i < 10; ++i) {
        unsigned char rnd[8];
        if (!fill_random_bytes(rnd, sizeof(rnd))) {
            close(dirfd);
            return false;
        }
        tmpname = ".impconf." + hex_encode(rnd, sizeof(rnd)) + ".tmp";
        fd = openat(dirfd, tmpname.c_str(),
                    O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
                    mode600);
        if (fd >= 0) break;
    }
    if (fd < 0) {
        close(dirfd);
        return false;
    }

    struct stat fs;
    if (fstat(fd, &fs) != 0 || !S_ISREG(fs.st_mode)) {
        close(fd);
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return false;
    }

    const char* data = "important_config";
    size_t len = strlen(data);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w <= 0) {
            close(fd);
            unlinkat(dirfd, tmpname.c_str(), 0);
            close(dirfd);
            return false;
        }
        off += (size_t)w;
    }

    if (fsync(fd) != 0) {
        // proceed but treat as failure for robustness
        close(fd);
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return false;
    }

    if (close(fd) != 0) {
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return false;
    }

    if (renameat(dirfd, tmpname.c_str(), dirfd, "important_config") != 0) {
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return false;
    }

    (void)fsync(dirfd); // best effort to persist directory entry
    close(dirfd);
    return true;
}

int main() {
    std::vector<std::string> tests = {
        ".",
        "cpp_out1",
        "cpp_out1/subdir",
        "cpp out 2",
        "cpp_out3"
    };
    for (const auto& d : tests) {
        // Create directories as needed
        std::string cmd = "mkdir -p -- ";
        cmd += "'" + d + "'";
        int r = system(cmd.c_str());
        (void)r;
        bool ok = write_important_config(d);
        std::cout << d << ": " << (ok ? "ok" : "fail") << std::endl;
    }
    return 0;
}