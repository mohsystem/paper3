#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

// Chain-of-Through Step 1: Problem understanding & function signature
// Open/create file with 0644 and write content, return bytes written or -1 on error.
long long secure_open_write(const char* filename, const char* content) {
    if (filename == NULL || content == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Chain-of-Through Step 2: Security requirements
    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = (mode_t)0644;

    // Chain-of-Through Step 3: Secure coding generation
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        return -1;
    }

    // Ensure it's a regular file
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }

    // Enforce exact permissions regardless of umask
    (void)fchmod(fd, mode);

    const char* buf = content;
    size_t len = strlen(content);
    size_t off = 0;

    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        off += (size_t)n;
    }

    (void)fsync(fd);
    close(fd);
    return (long long)off;
}

// Chain-of-Through Step 4 & 5 reflected through validations and safe operations.

int main(void) {
    const char* content = "important_config";
    const char* files[5] = {
        "important_config",
        "important_config_1",
        "important_config_2",
        "important_config", // re-open existing
        "important_config_readwrite"
    };

    for (int i = 0; i < 5; ++i) {
        long long written = secure_open_write(files[i], content);
        if (written >= 0) {
            printf("Test %d: Wrote %lld bytes to %s\n", i + 1, written, files[i]);
        } else {
            printf("Test %d failed for %s: %s\n", i + 1, files[i], strerror(errno));
        }
    }
    return 0;
}