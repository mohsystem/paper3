#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define MAX_READ_BYTES (10 * 1024 * 1024) /* 10 MB */

static char* read_from_safe_dir(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        errno = EINVAL;
        return NULL;
    }
    if (strchr(filename, '\0') == NULL) { /* always true, but keep for pattern parity */ }

    /* Disallow absolute paths */
    if (filename[0] == '/') {
        errno = EPERM;
        return NULL;
    }

    /* Resolve base safe directory */
    char base_real[PATH_MAX];
    if (realpath("/safe", base_real) == NULL) {
        /* Safe directory not found or not accessible */
        return NULL;
    }

    /* Build joined path safely */
    char joined[PATH_MAX];
    size_t base_len = strlen(base_real);
    size_t fn_len = strlen(filename);

    if (base_len + 1 + fn_len >= sizeof(joined)) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    snprintf(joined, sizeof(joined), "%s/%s", base_real, filename);

    /* Resolve the final path; this also resolves any symlinks */
    char resolved[PATH_MAX];
    if (realpath(joined, resolved) == NULL) {
        /* File does not exist or cannot resolve */
        return NULL;
    }

    /* Ensure resolved path is inside base_real */
    size_t res_len = strlen(resolved);
    if (res_len < base_len || strncmp(resolved, base_real, base_len) != 0 ||
        (res_len > base_len && resolved[base_len] != '/')) {
        errno = EPERM;
        return NULL;
    }

    /* Open file without following final symlink to avoid TOCTOU on final component */
    int fd = open(resolved, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        return NULL;
    }

    /* Verify it's a regular file */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return NULL;
    }

    if (st.st_size < 0 || st.st_size > MAX_READ_BYTES) {
        close(fd);
        errno = EFBIG;
        return NULL;
    }

    size_t size = (size_t)st.st_size;
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }

    size_t total = 0;
    while (total < size) {
        ssize_t n = read(fd, buf + total, size - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(buf);
            close(fd);
            return NULL;
        }
        if (n == 0) break;
        total += (size_t)n;
    }
    buf[total] = '\0';
    close(fd);
    return buf;
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "../etc/passwd",
        "/etc/passwd",
        "subdir/notes.txt",
        "symlink_to_secret"
    };

    for (int i = 0; i < 5; i++) {
        const char* t = tests[i];
        printf("Test %d (%s):\n", i + 1, t);
        char* content = read_from_safe_dir(t);
        if (content) {
            size_t len = strlen(content);
            if (len > 200) {
                fwrite(content, 1, 200, stdout);
                printf("...\n");
            } else {
                fwrite(content, 1, len, stdout);
                printf("\n");
            }
            free(content);
        } else {
            perror("Error");
        }
        printf("\n");
    }
    return 0;
}