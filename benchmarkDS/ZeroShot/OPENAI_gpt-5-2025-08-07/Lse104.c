#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static ssize_t write_important_config(const char* path, const char* content) {
    if (path == NULL || *path == '\0' || content == NULL) {
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
    int fd = open(path, flags, mode);
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

    const char* buf = content;
    size_t len = strlen(content);
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
            close(fd);
            errno = EIO;
            return -1;
        }
        total += (size_t)n;
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

    return (ssize_t)total;
}

int main(void) {
    const char* files[5] = {
        "c_test_secure_1.cfg",
        "c_test_secure_2.cfg",
        "c_test_secure_3.cfg",
        "c_test_secure_4.cfg",
        "c_test_secure_5.cfg"
    };
    for (int i = 0; i < 5; ++i) {
        ssize_t n = write_important_config(files[i], "important_config");
        if (n >= 0) {
            printf("C wrote %zd bytes to %s\n", n, files[i]);
        } else {
            printf("C failed for %s: %s\n", files[i], strerror(errno));
        }
    }
    return 0;
}