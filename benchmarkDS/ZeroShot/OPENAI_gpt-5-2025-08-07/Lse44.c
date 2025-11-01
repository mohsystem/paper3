#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE_BYTES (1024 * 1024) /* 1MB */

static int split_path(const char* s, char*** out_parts, size_t* out_count) {
    if (!s || !*s) return -1;
    if (s[0] == '/') return -1;

    // Count parts
    size_t count = 0;
    for (const char* p = s; *p;) {
        const char* q = strchr(p, '/');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        if (len == 0) return -1;
        if ((len == 1 && p[0] == '.') || (len == 2 && p[0] == '.' && p[1] == '.')) return -1;
        count++;
        if (!q) break;
        p = q + 1;
    }
    if (count == 0) return -1;

    char** parts = (char**)calloc(count, sizeof(char*));
    if (!parts) return -1;

    size_t idx = 0;
    for (const char* p = s; *p && idx < count;) {
        const char* q = strchr(p, '/');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        parts[idx] = (char*)malloc(len + 1);
        if (!parts[idx]) {
            for (size_t k = 0; k < idx; ++k) free(parts[k]);
            free(parts);
            return -1;
        }
        memcpy(parts[idx], p, len);
        parts[idx][len] = '\0';
        idx++;
        if (!q) break;
        p = q + 1;
    }

    *out_parts = parts;
    *out_count = count;
    return 0;
}

static void free_parts(char** parts, size_t count) {
    if (!parts) return;
    for (size_t i = 0; i < count; ++i) free(parts[i]);
    free(parts);
}

/* Returns malloc'ed buffer with file content and sets out_len; returns NULL on error */
char* readSafeFile(const char* requested, size_t* out_len) {
    if (!requested || !out_len) {
        errno = EINVAL;
        return NULL;
    }
    if (strchr(requested, '\0') != requested + strlen(requested)) {
        errno = EINVAL;
        return NULL;
    }

    char** parts = NULL;
    size_t part_count = 0;
    if (split_path(requested, &parts, &part_count) != 0) {
        errno = EINVAL;
        return NULL;
    }

    int dirfd = open("/safe", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        free_parts(parts, part_count);
        return NULL;
    }

    int cwd_fd = dirfd;
    int fd = -1;
    char* buffer = NULL;

    // Traverse directories
    for (size_t i = 0; i + 1 < part_count; ++i) {
        int next = openat(cwd_fd, parts[i], O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (next < 0) {
            if (cwd_fd != dirfd) close(cwd_fd);
            close(dirfd);
            free_parts(parts, part_count);
            return NULL;
        }
        if (cwd_fd != dirfd) close(cwd_fd);
        cwd_fd = next;
    }

    // Open final file
    fd = openat(cwd_fd, parts[part_count - 1], O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        free_parts(parts, part_count);
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        free_parts(parts, part_count);
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        free_parts(parts, part_count);
        errno = EPERM;
        return NULL;
    }
    if (st.st_size > (off_t)MAX_SIZE_BYTES) {
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        free_parts(parts, part_count);
        errno = EFBIG;
        return NULL;
    }

    size_t cap = (st.st_size > 0) ? (size_t)st.st_size : 0;
    if (cap == 0) cap = 1;
    buffer = (char*)malloc(cap + 1);
    if (!buffer) {
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        free_parts(parts, part_count);
        return NULL;
    }

    size_t total = 0;
    ssize_t n;
    while ((n = read(fd, buffer + total, cap - total)) > 0) {
        total += (size_t)n;
        if (total == cap) {
            if (cap >= MAX_SIZE_BYTES) {
                free(buffer);
                if (cwd_fd != dirfd) close(cwd_fd);
                close(dirfd);
                close(fd);
                free_parts(parts, part_count);
                errno = EFBIG;
                return NULL;
            }
            size_t new_cap = cap * 2;
            if (new_cap > MAX_SIZE_BYTES) new_cap = MAX_SIZE_BYTES;
            char* tmp = (char*)realloc(buffer, new_cap + 1);
            if (!tmp) {
                free(buffer);
                if (cwd_fd != dirfd) close(cwd_fd);
                close(dirfd);
                close(fd);
                free_parts(parts, part_count);
                return NULL;
            }
            buffer = tmp;
            cap = new_cap;
        }
    }
    if (n < 0) {
        free(buffer);
        if (cwd_fd != dirfd) close(cwd_fd);
        close(dirfd);
        close(fd);
        free_parts(parts, part_count);
        return NULL;
    }
    buffer[total] = '\0';
    *out_len = total;

    if (cwd_fd != dirfd) close(cwd_fd);
    close(dirfd);
    close(fd);
    free_parts(parts, part_count);
    return buffer;
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "../etc/passwd",
        "/etc/passwd",
        "subdir/file.txt",
        "symlink_outside.txt"
    };
    for (int i = 0; i < 5; ++i) {
        size_t len = 0;
        char* content = readSafeFile(tests[i], &len);
        if (content) {
            printf("Read OK: %s (len=%zu)\n", tests[i], len);
            free(content);
        } else {
            printf("Read FAIL: %s -> %s\n", tests[i], strerror(errno));
        }
    }
    return 0;
}