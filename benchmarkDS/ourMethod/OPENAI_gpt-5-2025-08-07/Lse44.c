#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

#define MAX_SIZE (4u * 1024u * 1024u) /* 4 MiB */
static const char* BASE_DIR = "/safe";

static int is_valid_name(const char* s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > 255) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

char* read_safe_file(const char* name, size_t* out_len) {
    if (out_len) *out_len = 0;
    if (!is_valid_name(name)) {
        return NULL;
    }

    int dirfd = open(BASE_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        return NULL;
    }

    int fd = openat(dirfd, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        close(dirfd);
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        close(dirfd);
        return NULL;
    }

    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dirfd);
        return NULL;
    }

    if (st.st_size < 0 || (unsigned long long)st.st_size > MAX_SIZE) {
        close(fd);
        close(dirfd);
        return NULL;
    }

    size_t sz = (size_t)st.st_size;
    size_t cap = sz + 1; /* for NUL termination */
    char* buf = (char*)malloc(cap);
    if (!buf) {
        close(fd);
        close(dirfd);
        return NULL;
    }

    size_t off = 0;
    while (off < sz) {
        ssize_t r = read(fd, buf + off, sz - off);
        if (r < 0) {
            free(buf);
            close(fd);
            close(dirfd);
            return NULL;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    buf[off] = '\0';
    if (out_len) *out_len = off;

    close(fd);
    close(dirfd);
    return buf;
}

static void print_hash(const unsigned char* data, size_t n) {
    /* simple 32-bit FNV-1a for test output (not cryptographic) */
    unsigned int h = 2166136261u;
    for (size_t i = 0; i < n; i++) {
        h ^= data[i];
        h *= 16777619u;
    }
    printf("%08x", h);
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "does_not_exist.txt",
        "../passwd",
        "valid-file_123",
        "bad\\name"
    };

    for (int i = 0; i < 5; i++) {
        size_t len = 0;
        char* data = read_safe_file(tests[i], &len);
        if (data) {
            printf("OK %s size=%zu hash=", tests[i], len);
            print_hash((const unsigned char*)data, len);
            printf("\n");
            free(data);
        } else {
            printf("ERR %s\n", tests[i]);
        }
    }
    return 0;
}