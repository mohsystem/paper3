#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>

ssize_t write_config_secure(const std::string& path, const std::string& content) {
    if (path.empty()) {
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

    ssize_t total = 0;
    const char* buf = content.c_str();
    size_t len = content.size();

    // Truncate existing file to avoid leftover bytes
    if (ftruncate(fd, 0) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }

    while (total < static_cast<ssize_t>(len)) {
        ssize_t n = write(fd, buf + total, len - total);
        if (n == -1) {
            if (errno == EINTR) continue;
            int saved = errno;
            close(fd);
            errno = saved;
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += n;
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

    return total;
}

int main() {
    const char* files[5] = {
        "lse104_cpp_test1.cfg",
        "lse104_cpp_test2.cfg",
        "lse104_cpp_test3.cfg",
        "lse104_cpp_test4.cfg",
        "lse104_cpp_test5.cfg"
    };
    for (int i = 0; i < 5; ++i) {
        ssize_t written = write_config_secure(files[i], std::string("important_config"));
        if (written >= 0) {
            std::cout << "Test " << (i+1) << " wrote " << written << " bytes to " << files[i] << std::endl;
        } else {
            std::cout << "Test " << (i+1) << " failed for " << files[i] << ": " << std::strerror(errno) << std::endl;
        }
    }
    return 0;
}