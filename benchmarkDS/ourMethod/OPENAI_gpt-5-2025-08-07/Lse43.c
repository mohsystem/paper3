#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10 MiB */
#define BASE_DIR "/safe"

static int is_valid_name(const char *name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len < 1 || len > 255) return 0;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/' || c == '\\' || c == '\0') return 0;
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

/* Returns 0 on success, -1 on failure. On success, caller must free(*out_buf). */
int read_from_safe(const char *name, unsigned char **out_buf, size_t *out_len) {
    int ret = -1;
    int dirfd = -1;
    int fd = -1;
    unsigned char *buf = NULL;
    size_t cap = 0;
    size_t total = 0;

    if (out_buf == NULL || out_len == NULL) {
        return -1;
    }
    *out_buf = NULL;
    *out_len = 0;

    if (!is_valid_name(name)) {
        return -1;
    }

    dirfd = open(BASE_DIR, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) {
        goto cleanup;
    }

    fd = openat(dirfd, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        goto cleanup;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        goto cleanup;
    }
    if (!S_ISREG(st.st_mode)) {
        goto cleanup;
    }
    if (st.st_size > 0 && st.st_size > (off_t)MAX_FILE_SIZE) {
        goto cleanup;
    }

    if (st.st_size > 0 && st.st_size <= (off_t)MAX_FILE_SIZE) {
        cap = (size_t)st.st_size;
    } else {
        cap = 8192;
    }
    if (cap > MAX_FILE_SIZE) cap = MAX_FILE_SIZE;

    buf = (unsigned char *)malloc(cap + 1);
    if (buf == NULL) {
        goto cleanup;
    }

    while (1) {
        if (total == cap) {
            size_t new_cap = cap * 2;
            if (new_cap < cap) goto cleanup; /* overflow check */
            if (new_cap > MAX_FILE_SIZE) new_cap = MAX_FILE_SIZE;
            if (new_cap == cap) goto cleanup;
            unsigned char *nbuf = (unsigned char *)realloc(buf, new_cap + 1);
            if (nbuf == NULL) {
                goto cleanup;
            }
            buf = nbuf;
            cap = new_cap;
        }
        size_t to_read = cap - total;
        if (to_read > 8192) to_read = 8192;
        ssize_t r = read(fd, buf + total, to_read);
        if (r < 0) {
            goto cleanup;
        }
        if (r == 0) break;
        total += (size_t)r;
        if (total > MAX_FILE_SIZE) {
            goto cleanup;
        }
    }

    buf[total] = '\0';
    *out_buf = buf;
    *out_len = total;
    buf = NULL;
    ret = 0;

cleanup:
    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);
    if (buf != NULL) {
        /* No sensitive data; direct free. */
        free(buf);
    }
    return ret;
}

int main(void) {
    const char *tests[5] = {
        "example.txt",
        "nonexistent.txt",
        "../passwd",
        "symlink",
        "nested.txt"
    };
    for (int i = 0; i < 5; i++) {
        unsigned char *data = NULL;
        size_t len = 0;
        if (read_from_safe(tests[i], &data, &len) == 0) {
            size_t preview_len = len < 64 ? len : 64;
            char preview[65];
            size_t j;
            for (j = 0; j < preview_len; j++) {
                char c = (char)data[j];
                preview[j] = (c == '\n') ? ' ' : c;
            }
            preview[preview_len] = '\0';
            printf("Read %s: %zu bytes; preview: %s\n", tests[i], len, preview);
            free(data);
        } else {
            printf("Read %s: error\n", tests[i]);
        }
    }
    return 0;
}