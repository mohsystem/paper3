#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static ssize_t write_important_config(const std::string& path, const std::string& content) {
    if (path.empty()) {
        errno = EINVAL;
        return -1;
    }

    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = 0600;
    int fd = open(path.c_str(), flags, mode);
    if (fd == -1) {
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EPERM;
        return -1;
    }

    const char* buf = content.c_str();
    size_t len = content.size();
    size_t total = 0;

    while (total < len) {
        ssize_t n = write(fd, buf + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            int saved = errno;
            close(fd);
            errno = saved;
            return -1;
        }
        if (n == 0) {
            // Should not happen for write; treat as error
            close(fd);
            errno = EIO;
            return -1;
        }
        total += static_cast<size_t>(n);
    }

    if (fsync(fd) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }

    if (close(fd) == -1) {
        return -1;
    }

    return static_cast<ssize_t>(total);
}

int main() {
    const char* files[5] = {
        "cpp_test_secure_1.cfg",
        "cpp_test_secure_2.cfg",
        "cpp_test_secure_3.cfg",
        "cpp_test_secure_4.cfg",
        "cpp_test_secure_5.cfg"
    };
    for (const char* f : files) {
        ssize_t n = write_important_config(std::string(f), std::string("important_config"));
        if (n >= 0) {
            std::cout << "C++ wrote " << n << " bytes to " << f << "\n";
        } else {
            std::cout << "C++ failed for " << f << ": " << std::strerror(errno) << "\n";
        }
    }
    return 0;
}