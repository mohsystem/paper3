#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

static bool is_safe_relative_component(const char* comp, size_t len) {
    if (len == 0) return false; // empty component
    if (len == 1 && comp[0] == '.') return false; // "."
    if (len == 2 && comp[0] == '.' && comp[1] == '.') return false; // ".."
    // Disallow any colon or backslash to avoid Windows drive or alt separators
    for (size_t i = 0; i < len; ++i) {
        if (comp[i] == ':' || comp[i] == '\\' || comp[i] == '\0') return false;
    }
    return true;
}

static bool validate_requested_path(const char* requested) {
    if (requested == NULL || requested[0] == '\0') return false;
    if (requested[0] == '/') return false; // absolute not allowed

    const char* p = requested;
    const char* start = p;
    while (1) {
        if (*p == '/' || *p == '\0') {
            size_t len = (size_t)(p - start);
            if (!is_safe_relative_component(start, len)) return false;
            if (*p == '\0') break;
            start = p + 1;
        }
        p++;
    }
    return true;
}

static int starts_with_dir(const char* base, const char* path) {
    size_t lb = strlen(base);
    size_t lp = strlen(path);
    if (lb > lp) return 0;
    if (strncmp(base, path, lb) != 0) return 0;
    if (lp == lb) return 1;
    // Ensure path boundary
    return path[lb] == '/';
}

char* read_file_from_safe_dir(const char* safe_dir, const char* requested) {
    if (safe_dir == NULL || requested == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (!validate_requested_path(requested)) {
        errno = EPERM;
        return NULL;
    }

    char base_real[PATH_MAX];
    if (realpath(safe_dir, base_real) == NULL) {
        // Safe dir must exist
        return NULL;
    }

    char joined[PATH_MAX];
    // Build joined path safely
    size_t need = strlen(base_real) + 1 + strlen(requested) + 1;
    if (need > sizeof(joined)) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    snprintf(joined, sizeof(joined), "%s/%s", base_real, requested);

    // Resolve target real path (requires it to exist)
    char target_real[PATH_MAX];
    if (realpath(joined, target_real) == NULL) {
        // file not found or other error
        return NULL;
    }

    if (!starts_with_dir(base_real, target_real)) {
        errno = EPERM;
        return NULL;
    }

    struct stat st_link;
    if (lstat(target_real, &st_link) != 0) {
        return NULL;
    }
    if (!S_ISREG(st_link.st_mode)) {
        errno = EPERM;
        return NULL;
    }
    // Disallow symlinks explicitly
    if (S_ISLNK(st_link.st_mode)) {
        errno = EPERM;
        return NULL;
    }

    FILE* f = fopen(target_real, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    size_t size = (size_t)sz;
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t rd = fread(buf, 1, size, f);
    fclose(f);
    if (rd != size) {
        free(buf);
        return NULL;
    }
    buf[size] = '\0';
    return buf;
}

static int make_dirs(const char* path) {
    // Simple mkdir -p for single-level relative directories
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return -1;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0700);
            *p = '/';
        }
    }
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) return -1;
    return 0;
}

static int write_text_file(const char* path, const char* content) {
    char dirbuf[PATH_MAX];
    snprintf(dirbuf, sizeof(dirbuf), "%s", path);
    char* last_slash = strrchr(dirbuf, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (make_dirs(dirbuf) != 0) return -1;
    }
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t len = strlen(content);
    size_t wr = fwrite(content, 1, len, f);
    fclose(f);
    return wr == len ? 0 : -1;
}

int main(void) {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd");
        return 1;
    }

    char base[PATH_MAX];
    snprintf(base, sizeof(base), "%s/%s", cwd, "safe_dir_c");
    if (make_dirs(base) != 0) {
        perror("make_dirs");
        return 1;
    }

    char file1[PATH_MAX], innerdir[PATH_MAX], innerfile[PATH_MAX], outside[PATH_MAX];
    snprintf(file1, sizeof(file1), "%s/file1.txt", base);
    snprintf(innerdir, sizeof(innerdir), "%s/sub", base);
    snprintf(innerfile, sizeof(innerfile), "%s/inner.txt", innerdir);
    snprintf(outside, sizeof(outside), "%s/../outside_c.txt", base);

    if (write_text_file(file1, "Hello from C file1") != 0) perror("write file1");
    if (write_text_file(innerfile, "Inner C file") != 0) perror("write inner");
    {
        char outside_abs[PATH_MAX];
        if (realpath(base, outside_abs)) {
            // create outside file in parent dir
            char parent[PATH_MAX];
            snprintf(parent, sizeof(parent), "%s/..", outside_abs);
            char parent_real[PATH_MAX];
            if (realpath(parent, parent_real)) {
                char outp[PATH_MAX];
                snprintf(outp, sizeof(outp), "%s/%s", parent_real, "outside_c.txt");
                write_text_file(outp, "Outside C file");
            }
        }
    }

    // Resolve base real for passing
    char base_real[PATH_MAX];
    if (!realpath(base, base_real)) {
        perror("realpath base");
        return 1;
    }

    // Test 1: normal file
    {
        char* s = read_file_from_safe_dir(base_real, "file1.txt");
        if (s) {
            printf("Test1 OK: %s\n", s);
            free(s);
        } else {
            printf("Test1 ERROR: %s\n", strerror(errno));
        }
    }

    // Test 2: subdirectory file
    {
        char* s = read_file_from_safe_dir(base_real, "sub/inner.txt");
        if (s) {
            printf("Test2 OK: %s\n", s);
            free(s);
        } else {
            printf("Test2 ERROR: %s\n", strerror(errno));
        }
    }

    // Test 3: traversal attempt
    {
        char* s = read_file_from_safe_dir(base_real, "../outside_c.txt");
        if (s) {
            printf("Test3 OK (unexpected): %s\n", s);
            free(s);
        } else {
            printf("Test3 Expected ERROR: %s\n", strerror(errno));
        }
    }

    // Test 4: absolute path attempt
    {
        char absfile[PATH_MAX];
        snprintf(absfile, sizeof(absfile), "%s/file1.txt", base_real);
        char* s = read_file_from_safe_dir(base_real, absfile);
        if (s) {
            printf("Test4 OK (unexpected): %s\n", s);
            free(s);
        } else {
            printf("Test4 Expected ERROR: %s\n", strerror(errno));
        }
    }

    // Test 5: missing file
    {
        char* s = read_file_from_safe_dir(base_real, "missing.txt");
        if (s) {
            printf("Test5 OK (unexpected): %s\n", s);
            free(s);
        } else {
            printf("Test5 Expected ERROR: %s\n", strerror(errno));
        }
    }

    return 0;
}