#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_NAME_LENGTH 255
#define SIZE_LIMIT (10 * 1024 * 1024) /* 10 MiB */

static bool is_simple_filename(const char *name) {
    if (name == NULL) return false;
    size_t len = strnlen(name, MAX_NAME_LENGTH + 2);
    if (len == 0 || len > MAX_NAME_LENGTH) return false;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return false;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (c == '/' || c == '\\' || c == '\0') return false;
    }
    return true;
}

static int open_base_dir(const char *base_dir) {
    if (base_dir == NULL || *base_dir == '\0') { errno = EINVAL; return -1; }
    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) return -1;
    struct stat st;
    if (fstat(dfd, &st) != 0) {
        int e = errno;
        close(dfd);
        errno = e;
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dfd);
        errno = ENOTDIR;
        return -1;
    }
    return dfd;
}

static ssize_t write_all(int fd, const void *buf, size_t len) {
    const unsigned char *p = (const unsigned char *)buf;
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, p + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)n;
    }
    if (fsync(fd) != 0) return -1;
    return (ssize_t)off;
}

/* Returns bytes written on success, -1 on failure */
int secure_create_new_file(const char *base_dir, const char *filename, const char *content) {
    if (!is_simple_filename(filename)) { errno = EINVAL; return -1; }
    int dfd = open_base_dir(base_dir);
    if (dfd < 0) return -1;

    int fd = openat(dfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) { close(dfd); return -1; }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno; close(fd); close(dfd); errno = e; return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd); close(dfd); errno = EINVAL; return -1;
    }

    size_t len = content ? strlen(content) : 0;
    if (len > SIZE_LIMIT) {
        close(fd); close(dfd); errno = EFBIG; return -1;
    }

    ssize_t wrote = 0;
    if (len > 0) {
        wrote = write_all(fd, content, len);
        if (wrote < 0) {
            int e = errno; close(fd); fsync(dfd); close(dfd); errno = e; return -1;
        }
    }

    close(fd);
    (void)fsync(dfd); /* best-effort directory sync */
    close(dfd);
    return (int)wrote;
}

/* Returns bytes written on success, -1 on failure */
int secure_truncate_and_write_file(const char *base_dir, const char *filename, const char *content) {
    if (!is_simple_filename(filename)) { errno = EINVAL; return -1; }
    int dfd = open_base_dir(base_dir);
    if (dfd < 0) return -1;

    int fd = openat(dfd, filename, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) { close(dfd); return -1; }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno; close(fd); close(dfd); errno = e; return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd); close(dfd); errno = EINVAL; return -1;
    }

    size_t len = content ? strlen(content) : 0;
    if (len > SIZE_LIMIT) {
        close(fd); close(dfd); errno = EFBIG; return -1;
    }

    ssize_t wrote = 0;
    if (len > 0) {
        wrote = write_all(fd, content, len);
        if (wrote < 0) {
            int e = errno; close(fd); fsync(dfd); close(dfd); errno = e; return -1;
        }
    }

    close(fd);
    (void)fsync(dfd); /* best-effort directory sync */
    close(dfd);
    return (int)wrote;
}

int main(void) {
    char tmpl[] = "/tmp/secure_open_demo_XXXXXX";
    char *base = mkdtemp(tmpl);
    if (!base) {
        printf("fatal\n");
        return 0;
    }

    int r1 = secure_create_new_file(base, "a.txt", "hello");
    printf("Test1 create_new a.txt bytes=%d\n", (r1 >= 0) ? r1 : -1);

    int r2 = secure_create_new_file(base, "a.txt", "x");
    if (r2 >= 0) printf("Test2 unexpected success\n");
    else printf("Test2 expected failure\n");

    int r3 = secure_truncate_and_write_file(base, "a.txt", "world");
    printf("Test3 truncate a.txt bytes=%d\n", (r3 >= 0) ? r3 : -1);

    int r4 = secure_create_new_file(base, "b.txt", "data");
    printf("Test4 create_new b.txt bytes=%d\n", (r4 >= 0) ? r4 : -1);

    int r5 = secure_create_new_file(base, "../evil.txt", "bad");
    if (r5 >= 0) printf("Test5 unexpected success\n");
    else printf("Test5 expected failure\n");

    return 0;
}