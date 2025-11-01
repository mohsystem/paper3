#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

ssize_t write_config_secure(const char* path, const char* content) {
    if (path == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (content == NULL) {
        content = "";
    }

    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = 0600;
    int fd = open(path, flags, mode);
    if (fd == -1) {
        return -1;
    }

    size_t len = strlen(content);
    ssize_t total = 0;

    if (ftruncate(fd, 0) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }

    while ((size_t)total < len) {
        ssize_t n = write(fd, content + total, len - total);
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

int main(void) {
    const char* files[5] = {
        "lse104_c_test1.cfg",
        "lse104_c_test2.cfg",
        "lse104_c_test3.cfg",
        "lse104_c_test4.cfg",
        "lse104_c_test5.cfg"
    };

    for (int i = 0; i < 5; ++i) {
        ssize_t written = write_config_secure(files[i], "important_config");
        if (written >= 0) {
            printf("Test %d wrote %zd bytes to %s\n", i + 1, written, files[i]);
        } else {
            printf("Test %d failed for %s: %s\n", i + 1, files[i], strerror(errno));
        }
    }
    return 0;
}