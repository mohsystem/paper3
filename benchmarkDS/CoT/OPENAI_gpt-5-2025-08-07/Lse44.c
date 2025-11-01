/* 
 Step 1: Read a requested file from /safe directory
 Step 2: Security: prevent traversal, enforce base boundary, avoid symlink escape, O_NOFOLLOW, size limit
 Step 3/5: Secure coding and output
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BYTES (5 * 1024 * 1024) /* 5 MiB */

static void sanitize_relative(const char* in, char* out, size_t out_sz) {
    /* Convert backslashes to slashes and strip leading slashes to keep relative */
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < out_sz; ++i) {
        char c = in[i] == '\\' ? '/' : in[i];
        if (j == 0 && c == '/') {
            continue; /* skip leading slashes */
        }
        out[j++] = c;
    }
    out[j] = '\0';
}

char* read_safe_file(const char* requested_path, size_t* out_len) {
    if (!requested_path) {
        errno = EINVAL;
        return NULL;
    }

    char base_real[PATH_MAX];
    if (!realpath("/safe", base_real)) {
        return NULL; /* errno set by realpath */
    }

    char rel[PATH_MAX];
    sanitize_relative(requested_path, rel, sizeof(rel));

    char combined[PATH_MAX];
    if (snprintf(combined, sizeof(combined), "%s/%s", base_real, rel) >= (int)sizeof(combined)) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    char target_real[PATH_MAX];
    if (!realpath(combined, target_real)) {
        return NULL; /* errno set by realpath */
    }

    size_t base_len = strlen(base_real);
    if (strncmp(target_real, base_real, base_len) != 0 ||
        !(target_real[base_len] == '/' || target_real[base_len] == '\0')) {
        errno = EACCES;
        return NULL;
    }

    struct stat st;
    if (lstat(target_real, &st) != 0) {
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        errno = EISDIR;
        return NULL;
    }
    if (st.st_size < 0 || (size_t)st.st_size > MAX_BYTES) {
        errno = EFBIG;
        return NULL;
    }

    int fd = open(target_real, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        return NULL;
    }

    struct stat st2;
    if (fstat(fd, &st2) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return NULL;
    }
    if (!S_ISREG(st2.st_mode)) {
        close(fd);
        errno = EINVAL;
        return NULL;
    }
    if ((size_t)st2.st_size > MAX_BYTES) {
        close(fd);
        errno = EFBIG;
        return NULL;
    }

    size_t to_read = (size_t)st2.st_size;
    char* buf = (char*)malloc(to_read + 1);
    if (!buf) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }

    size_t off = 0;
    while (off < to_read) {
        ssize_t r = read(fd, buf + off, to_read - off);
        if (r < 0) {
            int e = errno;
            free(buf);
            close(fd);
            errno = e;
            return NULL;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    buf[off] = '\0';
    if (out_len) *out_len = off;
    return buf;
}

int main(void) {
    /* Step 4: Code review done, testing with 5 cases */
    const char* tests[5] = {
        "example.txt",
        "subdir/data.txt",
        "../etc/passwd",
        "/safe/../safe/secret.txt",
        "nonexistent.txt"
    };

    for (int i = 0; i < 5; ++i) {
        printf("=== Test: %s ===\n", tests[i]);
        size_t len = 0;
        char* content = read_safe_file(tests[i], &len);
        if (!content) {
            printf("Error: %s\n", strerror(errno));
        } else {
            printf("Read OK. Length: %zu\n", len);
            free(content);
        }
    }
    return 0;
}