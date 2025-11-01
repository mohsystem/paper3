/* Chain-of-Through:
 * 1) Purpose: safely read a file from /safe directory (or provided base).
 * 2) Security: validate filename, prevent traversal, ensure path within base, reject symlinks, enforce size limit.
 * 3) Implementation: use realpath for base, lstat/open with O_NOFOLLOW, strict filename whitelist.
 * 4) Review: check all error paths, free resources.
 * 5) Output: secure final C code below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#define MAX_BYTES (10 * 1024 * 1024) /* 10 MB */
#define FIXED_BASE_DIR "/safe"

/* Validate filename: only A-Z a-z 0-9 . _ - and length 1..255, no slashes */
static int valid_filename(const char* filename) {
    if (!filename) return 0;
    size_t len = strlen(filename);
    if (len == 0 || len > 255) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = filename[i];
        if ( !( (c>='A'&&c<='Z') || (c>='a'&&c<='z') || (c>='0'&&c<='9') || c=='.' || c=='_' || c=='-' ) ) {
            return 0;
        }
    }
    return 1;
}

/* Secure read from base directory; returns malloc'd string (UTF-8 assumed), or NULL on error */
char* read_from_base(const char* base_dir, const char* filename, size_t* out_len) {
    if (out_len) *out_len = 0;

    if (!base_dir || !*base_dir || !filename || !*filename) {
        errno = EINVAL;
        return NULL;
    }
    if (!valid_filename(filename)) {
        errno = EPERM;
        return NULL;
    }

    /* Resolve base directory */
    char base_resolved[PATH_MAX];
    if (!realpath(base_dir, base_resolved)) {
        /* If base doesn't exist, try to use absolute normalized path (fallback) */
        if (errno != ENOENT) return NULL;
        /* Normalize by combining getcwd and base_dir if relative */
        if (base_dir[0] == '/') {
            strncpy(base_resolved, base_dir, sizeof(base_resolved)-1);
            base_resolved[sizeof(base_resolved)-1] = '\0';
        } else {
            char cwd[PATH_MAX];
            if (!getcwd(cwd, sizeof(cwd))) return NULL;
            snprintf(base_resolved, sizeof(base_resolved), "%s/%s", cwd, base_dir);
        }
    }

    /* Build target path string */
    char target_path[PATH_MAX];
    if (snprintf(target_path, sizeof(target_path), "%s/%s", base_resolved, filename) >= (int)sizeof(target_path)) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    /* lstat to ensure it's not a symlink and is regular file */
    struct stat st;
    if (lstat(target_path, &st) != 0) {
        return NULL; /* errno set */
    }
    if (S_ISLNK(st.st_mode)) {
        errno = EPERM; /* symbolic links not allowed */
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        errno = EISDIR; /* not a regular file */
        return NULL;
    }
    if (st.st_size < 0 || st.st_size > MAX_BYTES) {
        errno = EFBIG; /* too large */
        return NULL;
    }

    /* Ensure resolved canonical path is still under base (defense-in-depth) */
    char target_real[PATH_MAX];
    if (!realpath(target_path, target_real)) {
        return NULL;
    }
    size_t base_len = strlen(base_resolved);
    if (strncmp(target_real, base_resolved, base_len) != 0 ||
        (target_real[base_len] != '/' && target_real[base_len] != '\0')) {
        errno = EPERM;
        return NULL;
    }

    /* Open without following symlinks if possible */
    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = open(target_real, flags);
    if (fd < 0) {
        return NULL;
    }

    /* Read with cap */
    size_t cap = (size_t)st.st_size;
    if (cap > MAX_BYTES) { close(fd); errno = EFBIG; return NULL; }
    if (cap == 0) cap = 1; /* ensure malloc at least 1 */

    char* buf = (char*)malloc(cap + 1);
    if (!buf) { close(fd); errno = ENOMEM; return NULL; }

    size_t total = 0;
    while (1) {
        ssize_t n = read(fd, buf + total, cap - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(buf);
            close(fd);
            return NULL;
        }
        if (n == 0) break;
        total += (size_t)n;
        if (total > MAX_BYTES) {
            free(buf);
            close(fd);
            errno = EFBIG;
            return NULL;
        }
        if (total == cap) {
            size_t new_cap = cap * 2;
            if (new_cap < cap || new_cap > MAX_BYTES) new_cap = MAX_BYTES;
            char* nb = (char*)realloc(buf, new_cap + 1);
            if (!nb) {
                free(buf);
                close(fd);
                errno = ENOMEM;
                return NULL;
            }
            buf = nb;
            cap = new_cap;
        }
    }
    close(fd);
    buf[total] = '\0';
    if (out_len) *out_len = total;
    return buf;
}

/* Fixed base wrapper */
char* read_from_safe_dir(const char* filename, size_t* out_len) {
    return read_from_base(FIXED_BASE_DIR, filename, out_len);
}

/* Helper for tests: print result or error */
static void print_result(const char* label, char* (*fn)(void), int free_needed) {
    printf("== %s ==\n", label);
    char* res = fn();
    if (!res) {
        printf("Error: %s\n", strerror(errno));
    } else {
        printf("%s\n", res[0] ? res : "(empty)");
        if (free_needed) free(res);
    }
}

/* Test harness with 5 cases */
int main(void) {
    /* Prepare controlled temp base */
    char templ[] = "/tmp/safe_c_tests_XXXXXX";
    char* base_dir = mkdtemp(templ);
    if (!base_dir) {
        fprintf(stderr, "Failed to create temp dir: %s\n", strerror(errno));
        return 1;
    }

    /* Create hello.txt */
    char hello_path[PATH_MAX];
    snprintf(hello_path, sizeof(hello_path), "%s/hello.txt", base_dir);
    FILE* f = fopen(hello_path, "wb");
    if (f) { fputs("Hello from C", f); fclose(f); }

    /* Create directory not_a_file */
    char dir_path[PATH_MAX];
    snprintf(dir_path, sizeof(dir_path), "%s/not_a_file", base_dir);
    mkdir(dir_path, 0700);

    size_t out_len = 0;

    /* Lambdas-like shims for tests using static functions */

    /* Test 1: Valid read */
    char* t1(void) {
        return read_from_base(base_dir, "hello.txt", &out_len);
    }
    print_result("Test 1: valid read", t1, 1);

    /* Test 2: Traversal blocked */
    char* t2(void) {
        return read_from_base(base_dir, "../etc/passwd", &out_len);
    }
    print_result("Test 2: traversal blocked", t2, 0);

    /* Test 3: Invalid chars */
    char* t3(void) {
        return read_from_base(base_dir, "bad/name.txt", &out_len);
    }
    print_result("Test 3: invalid chars blocked", t3, 0);

    /* Test 4: Missing file */
    char* t4(void) {
        return read_from_base(base_dir, "missing.txt", &out_len);
    }
    print_result("Test 4: non-existing file", t4, 0);

    /* Test 5: Directory instead of file */
    char* t5(void) {
        return read_from_base(base_dir, "not_a_file", &out_len);
    }
    print_result("Test 5: directory instead of file", t5, 0);

    /* Demo: fixed /safe (may fail) */
    char* t6(void) {
        return read_from_safe_dir("example.txt", &out_len);
    }
    print_result("Demo: fixed /safe read (may fail)", t6, 1);

    return 0;
}