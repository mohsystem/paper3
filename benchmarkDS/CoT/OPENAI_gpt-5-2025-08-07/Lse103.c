#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

static int safe_write_all(int fd, const void* buf, size_t len) {
    const char* p = (const char*)buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, p + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        total += (size_t)n;
    }
    return 0;
}

// O_CREAT | O_WRONLY | O_TRUNC
int create_or_truncate_write(const char* path, const char* data) {
    int flags = O_CREAT | O_WRONLY | O_TRUNC;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = 0600;
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return -1;
    }
    int rc = safe_write_all(fd, data, strlen(data));
    if (rc == 0) {
        (void)fsync(fd);
    }
    int err = errno;
    close(fd);
    if (rc != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

// O_CREAT | O_WRONLY | O_EXCL
int create_exclusive_write(const char* path, const char* data) {
    int flags = O_CREAT | O_WRONLY | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = 0600;
    int fd = open(path, flags, mode);
    if (fd < 0) {
        if (errno == EEXIST) return 1; // exists
        return -1;
    }
    int rc = safe_write_all(fd, data, strlen(data));
    if (rc == 0) {
        (void)fsync(fd);
    }
    int err = errno;
    close(fd);
    if (rc != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

static size_t read_all_len(const char* path) {
    int fd = open(path, O_RDONLY
#ifdef O_CLOEXEC
    | O_CLOEXEC
#endif
    );
    if (fd < 0) return 0;
    char buf[4096];
    size_t total = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        total += (size_t)n;
    }
    close(fd);
    return total;
}

int main(void) {
    pid_t pid = getpid();
    long long t = (long long)time(NULL);
    char pathA[256];
    char pathB[256];
    snprintf(pathA, sizeof(pathA), "/tmp/lse103_c_A_%d_%lld.txt", (int)pid, t);
    snprintf(pathB, sizeof(pathB), "/tmp/lse103_c_B_%d_%lld.txt", (int)pid, t);

    int r1 = create_exclusive_write(pathA, "hello exclusive A");
    printf("T1 ex-create A: %d\n", r1);

    int r2 = create_exclusive_write(pathA, "should fail");
    printf("T2 ex-create A again: %d\n", r2);

    int r3 = create_or_truncate_write(pathA, "truncated A");
    printf("T3 truncate-write A: %d len=%zu\n", r3, read_all_len(pathA));

    int r4 = create_or_truncate_write(pathB, "new B");
    printf("T4 create-or-truncate B: %d len=%zu\n", r4, read_all_len(pathB));

    int r5 = create_exclusive_write(pathB, "should fail B");
    printf("T5 ex-create B again: %d\n", r5);

    unlink(pathA);
    unlink(pathB);
    return 0;
}