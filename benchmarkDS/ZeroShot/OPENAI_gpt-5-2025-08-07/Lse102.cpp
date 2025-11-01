#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static std::string secure_open_and_write(const std::string& filename, const std::string& content) {
    struct stat lst;
    if (lstat(filename.c_str(), &lst) == 0) {
        if (S_ISLNK(lst.st_mode)) {
            throw std::runtime_error("Refusing to operate on a symbolic link: " + filename);
        }
    }

    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 0644
    int fd = open(filename.c_str(), flags, mode);
    if (fd < 0) {
        throw std::runtime_error(std::string("open failed: ") + std::strerror(errno));
    }

    // Enforce permissions (best effort)
    (void)fchmod(fd, mode);

    // Truncate to write exactly content
    if (ftruncate(fd, 0) != 0) {
        int e = errno;
        close(fd);
        throw std::runtime_error(std::string("ftruncate failed: ") + std::strerror(e));
    }

    // Write loop
    const char* buf = content.data();
    size_t len = content.size();
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            int e = errno;
            close(fd);
            throw std::runtime_error(std::string("write failed: ") + std::strerror(e));
        }
        off += static_cast<size_t>(n);
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        int e = errno;
        close(fd);
        throw std::runtime_error(std::string("lseek failed: ") + std::strerror(e));
    }

    std::string result;
    char rbuf[4096];
    for (;;) {
        ssize_t n = read(fd, rbuf, sizeof(rbuf));
        if (n < 0) {
            if (errno == EINTR) continue;
            int e = errno;
            close(fd);
            throw std::runtime_error(std::string("read failed: ") + std::strerror(e));
        }
        if (n == 0) break;
        result.append(rbuf, rbuf + n);
    }

    close(fd);
    return result;
}

int main() {
    try {
        const std::string filename = "important_config";
        const std::string content = "important_config";
        for (int i = 0; i < 5; ++i) {
            std::string out = secure_open_and_write(filename, content);
            std::cout << "Test " << (i + 1) << ": " << out << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}