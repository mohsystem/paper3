#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_SIZE (5 * 1024 * 1024) /* 5 MiB */
static const char* BASE_DIR = "static";

typedef struct {
    unsigned char* data;
    size_t size;
    int status; /* 0=ok, nonzero=error */
} ImgResult;

static int valid_name(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len < 1 || len > 128) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        int ok = (c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

static ImgResult img(const char* fileParam) {
    ImgResult res;
    res.data = NULL;
    res.size = 0;
    res.status = -1;

    if (!valid_name(fileParam)) {
        res.status = EINVAL;
        return res;
    }

    int dirfd = -1;
    int fd = -1;

    do {
        if (mkdir(BASE_DIR, 0700) != 0 && errno != EEXIST) { res.status = errno; break; }

        dirfd = open(BASE_DIR, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
        if (dirfd < 0) { res.status = errno ? errno : -1; break; }

        int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
        flags |= O_NOFOLLOW;
#endif
        fd = openat(dirfd, fileParam, flags);
        if (fd < 0) { res.status = errno ? errno : -1; break; }

        struct stat st;
        if (fstat(fd, &st) != 0) { res.status = errno ? errno : -1; break; }
        if (!S_ISREG(st.st_mode)) { res.status = EINVAL; break; }
        if (st.st_size < 0 || st.st_size > MAX_SIZE) { res.status = EFBIG; break; }

        res.size = (size_t)st.st_size;
        if (res.size == 0) {
            res.data = (unsigned char*)malloc(1); /* allocate minimal */
            if (res.data == NULL) { res.status = ENOMEM; break; }
            res.status = 0;
            break;
        }

        res.data = (unsigned char*)malloc(res.size);
        if (res.data == NULL) { res.status = ENOMEM; break; }

        size_t off = 0;
        while (off < res.size) {
            ssize_t r = read(fd, res.data + off, res.size - off);
            if (r < 0) { res.status = errno ? errno : -1; break; }
            if (r == 0) { res.status = EIO; break; }
            off += (size_t)r;
        }
        if (off != res.size) {
            break;
        }
        res.status = 0;
    } while (0);

    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);

    if (res.status != 0) {
        if (res.data != NULL) {
            free(res.data);
            res.data = NULL;
        }
        res.size = 0;
    }

    return res;
}

/* Helpers for tests */
static int ensure_dir(const char* path) {
    if (mkdir(path, 0700) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

static int write_file(const char* name, const unsigned char* data, size_t len) {
    if (ensure_dir(BASE_DIR) != 0) return -1;
    int dirfd = open(BASE_DIR, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) { close(dirfd); return -1; }
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) { close(fd); close(dirfd); return -1; }
        off += (size_t)w;
    }
    close(fd);
    close(dirfd);
    return 0;
}

static int write_big(const char* name, size_t size) {
    if (ensure_dir(BASE_DIR) != 0) return -1;
    int dirfd = open(BASE_DIR, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) { close(dirfd); return -1; }
    if (size > 0) {
        if (lseek(fd, (off_t)size - 1, SEEK_SET) < 0) { close(fd); close(dirfd); return -1; }
        if (write(fd, "", 1) != 1) { close(fd); close(dirfd); return -1; }
    }
    close(fd);
    close(dirfd);
    return 0;
}

int main(void) {
    ensure_dir(BASE_DIR);
    write_file("hello.txt", (const unsigned char*)"Hello, world!", 13);
    write_file("empty.bin", (const unsigned char*)"", 0);
    write_big("big.bin", MAX_SIZE + 1024);

#ifdef __unix__
    char linkPath[256];
    snprintf(linkPath, sizeof(linkPath), "%s/%s", BASE_DIR, "link.txt");
    unlink(linkPath);
    symlink("/etc/hosts", linkPath); /* ignore errors */
#endif

    /* 5 test cases */

    /* 1) valid small file */
    ImgResult r1 = img("hello.txt");
    if (r1.status == 0) {
        printf("Test1 hello.txt size=%zu\n", r1.size);
        free(r1.data);
    } else {
        printf("Test1 failed: %d\n", r1.status);
    }

    /* 2) empty file */
    ImgResult r2 = img("empty.bin");
    if (r2.status == 0) {
        printf("Test2 empty.bin size=%zu\n", r2.size);
        free(r2.data);
    } else {
        printf("Test2 failed: %d\n", r2.status);
    }

    /* 3) traversal attempt */
    ImgResult r3 = img("../etc/passwd");
    if (r3.status == 0) {
        printf("Test3 traversal size=%zu\n", r3.size);
        free(r3.data);
    } else {
        printf("Test3 expected failure: %d\n", r3.status);
    }

    /* 4) symlink attempt */
    ImgResult r4 = img("link.txt");
    if (r4.status == 0) {
        printf("Test4 symlink size=%zu\n", r4.size);
        free(r4.data);
    } else {
        printf("Test4 expected failure or skipped: %d\n", r4.status);
    }

    /* 5) oversized file */
    ImgResult r5 = img("big.bin");
    if (r5.status == 0) {
        printf("Test5 big.bin size=%zu\n", r5.size);
        free(r5.data);
    } else {
        printf("Test5 expected failure: %d\n", r5.status);
    }

    return 0;
}