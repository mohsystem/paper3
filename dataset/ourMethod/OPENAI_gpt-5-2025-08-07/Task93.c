#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

typedef struct {
    char* key;
    char* val;
} kv_pair;

static char* str_trim_dup(const char* s) {
    if (!s) return NULL;
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    if (len > 0) memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int cmp_pairs(const void* a, const void* b) {
    const kv_pair* pa = (const kv_pair*)a;
    const kv_pair* pb = (const kv_pair*)b;
    int c = strcmp(pa->key, pb->key);
    if (c != 0) return c;
    return strcmp(pa->val, pb->val);
}

static ssize_t read_line_dynamic(FILE* f, char** out_line) {
    size_t cap = 256;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) return -1;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            size_t ncap = cap * 2;
            if (ncap > 8192) { // limit line size
                free(buf);
                *out_line = NULL;
                // consume until end of line
                while (c != '\n' && c != EOF) {
                    c = fgetc(f);
                }
                return 0; // indicate skip
            }
            char* tmp = (char*)realloc(buf, ncap);
            if (!tmp) {
                free(buf);
                return -1;
            }
            buf = tmp;
            cap = ncap;
        }
        if (c == '\r') {
            int next = fpeek(f);
            (void)next;
            continue;
        }
        if (c == '\n') {
            break;
        }
        buf[len++] = (char)c;
    }
    if (ferror(f)) {
        free(buf);
        return -1;
    }
    if (len == 0 && c == EOF) {
        free(buf);
        return -2; // EOF no data
    }
    buf[len] = '\0';
    *out_line = buf;
    return (ssize_t)len;
}

static int starts_with(const char* s, const char* prefix) {
    size_t ls = strlen(s), lp = strlen(prefix);
    if (lp > ls) return 0;
    return strncmp(s, prefix, lp) == 0;
}

static int path_join_and_validate(const char* base_dir, const char* rel, char* out_path, size_t out_sz, char* errbuf, size_t errbuf_sz) {
    if (!base_dir || !rel || !out_path) {
        snprintf(errbuf, errbuf_sz, "Invalid arguments");
        return -1;
    }
    if (rel[0] == '/') {
        snprintf(errbuf, errbuf_sz, "Absolute paths not allowed");
        return -1;
    }

    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        snprintf(errbuf, errbuf_sz, "Base directory not found");
        return -1;
    }

    char candidate[PATH_MAX];
    if (snprintf(candidate, sizeof(candidate), "%s/%s", base_real, rel) >= (int)sizeof(candidate)) {
        snprintf(errbuf, errbuf_sz, "Path too long");
        return -1;
    }

    struct stat lst;
    if (lstat(candidate, &lst) != 0) {
        snprintf(errbuf, errbuf_sz, "File not found");
        return -1;
    }
    if (S_ISLNK(lst.st_mode)) {
        snprintf(errbuf, errbuf_sz, "Refusing to process symlink");
        return -1;
    }
    if (!S_ISREG(lst.st_mode)) {
        snprintf(errbuf, errbuf_sz, "Not a regular file");
        return -1;
    }

    char cand_real[PATH_MAX];
    if (!realpath(candidate, cand_real)) {
        snprintf(errbuf, errbuf_sz, "Failed to resolve realpath");
        return -1;
    }
    if (!starts_with(cand_real, base_real) || (cand_real[strlen(base_real)] != '/' && cand_real[strlen(base_real)] != '\0')) {
        snprintf(errbuf, errbuf_sz, "Resolved path escapes base directory");
        return -1;
    }

    if (strlen(cand_real) + 1 > out_sz) {
        snprintf(errbuf, errbuf_sz, "Output buffer too small");
        return -1;
    }
    strcpy(out_path, cand_real);
    return 0;
}

int read_and_sort_key_value_file(
    const char* base_dir,
    const char* relative_path,
    char*** out_keys,
    char*** out_vals,
    size_t* out_count,
    char* errbuf,
    size_t errbuf_sz
) {
    if (!out_keys || !out_vals || !out_count) {
        if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "Output pointers must not be NULL");
        return -1;
    }
    *out_keys = NULL;
    *out_vals = NULL;
    *out_count = 0;

    char path[PATH_MAX];
    if (path_join_and_validate(base_dir, relative_path, path, sizeof(path), errbuf, errbuf_sz) != 0) {
        return -1;
    }

    int fd = open(path, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "Failed to open file: %s", strerror(errno));
        return -1;
    }
    FILE* f = fdopen(fd, "rb");
    if (!f) {
        if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "fdopen failed");
        close(fd);
        return -1;
    }

    kv_pair* arr = NULL;
    size_t cap = 0, len = 0;

    for (;;) {
        char* line = NULL;
        ssize_t r = read_line_dynamic(f, &line);
        if (r == -2) { // EOF
            break;
        } else if (r == -1) {
            if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "Failed to read line");
            fclose(f);
            // free array
            for (size_t i = 0; i < len; i++) {
                free(arr[i].key);
                free(arr[i].val);
            }
            free(arr);
            return -1;
        } else if (r == 0) {
            // skipped long line
            continue;
        }

        // process line
        char* trimmed = str_trim_dup(line);
        free(line);
        if (!trimmed) continue;
        if (trimmed[0] == '\0' || trimmed[0] == '#' || trimmed[0] == ';') {
            free(trimmed);
            continue;
        }
        char* eq = strchr(trimmed, '=');
        if (!eq || eq == trimmed || *(eq + 1) == '\0') {
            free(trimmed);
            continue;
        }
        *eq = '\0';
        char* k = str_trim_dup(trimmed);
        char* v = str_trim_dup(eq + 1);
        free(trimmed);
        if (!k || !v) {
            free(k); free(v);
            continue;
        }
        if (strlen(k) == 0 || strlen(v) == 0 || strlen(k) > 1024 || strlen(v) > 4096) {
            free(k); free(v);
            continue;
        }

        if (len == cap) {
            size_t ncap = cap ? cap * 2 : 16;
            kv_pair* tmp = (kv_pair*)realloc(arr, ncap * sizeof(kv_pair));
            if (!tmp) {
                free(k); free(v);
                // cleanup
                for (size_t i = 0; i < len; i++) {
                    free(arr[i].key);
                    free(arr[i].val);
                }
                free(arr);
                fclose(f);
                if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "Out of memory");
                return -1;
            }
            arr = tmp;
            cap = ncap;
        }
        arr[len].key = k;
        arr[len].val = v;
        len++;
    }
    fclose(f);

    qsort(arr, len, sizeof(kv_pair), cmp_pairs);

    char** keys = (char**)calloc(len, sizeof(char*));
    char** vals = (char**)calloc(len, sizeof(char*));
    if ((!keys && len > 0) || (!vals && len > 0)) {
        if (keys) free(keys);
        if (vals) free(vals);
        for (size_t i = 0; i < len; i++) { free(arr[i].key); free(arr[i].val); }
        free(arr);
        if (errbuf && errbuf_sz) snprintf(errbuf, errbuf_sz, "Out of memory");
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        keys[i] = arr[i].key;
        vals[i] = arr[i].val;
    }
    free(arr);

    *out_keys = keys;
    *out_vals = vals;
    *out_count = len;
    if (errbuf && errbuf_sz) errbuf[0] = '\0';
    return 0;
}

static int secure_write_file(const char* base_dir, const char* name, const char* const* lines, size_t nlines, char* errbuf, size_t errbuf_sz) {
    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        snprintf(errbuf, errbuf_sz, "Base directory not found");
        return -1;
    }
    if (!name || !*name) {
        snprintf(errbuf, errbuf_sz, "Invalid file name");
        return -1;
    }
    char target[PATH_MAX];
    if (snprintf(target, sizeof(target), "%s/%s", base_real, name) >= (int)sizeof(target)) {
        snprintf(errbuf, errbuf_sz, "Target path too long");
        return -1;
    }
    // Ensure parent directories exist
    char tmp[PATH_MAX];
    strncpy(tmp, target, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    char* slash = strrchr(tmp, '/');
    if (slash) {
        *slash = '\0';
        // mkdir -p like
        char* p = tmp;
        while (*p) {
            if (*p == '/') {
                *p = '\0';
                if (strlen(tmp) > 0) mkdir(tmp, 0700);
                *p = '/';
            }
            p++;
        }
        if (strlen(tmp) > 0) mkdir(tmp, 0700);
    }

    int fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        snprintf(errbuf, errbuf_sz, "Failed to create file: %s", strerror(errno));
        return -1;
    }
    for (size_t i = 0; i < nlines; i++) {
        const char* line = lines[i];
        size_t len = strlen(line);
        if (write(fd, line, len) != (ssize_t)len) {
            close(fd);
            snprintf(errbuf, errbuf_sz, "Failed to write");
            return -1;
        }
        if (write(fd, "\n", 1) != 1) {
            close(fd);
            snprintf(errbuf, errbuf_sz, "Failed to write newline");
            return -1;
        }
    }
    fsync(fd);
    close(fd);
    return 0;
}

int main(void) {
    char tmpdir[PATH_MAX];
    // Create temp directory
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/task93_kv_c_%ld", (long)time(NULL));
    mkdir(tmpdir, 0700);

    char err[256];

    // Test 1: Normal case
    const char* t1[] = {"b=2", "a=1", "c=3"};
    secure_write_file(tmpdir, "test1.txt", t1, 3, err, sizeof(err));
    char** keys = NULL;
    char** vals = NULL;
    size_t count = 0;
    if (read_and_sort_key_value_file(tmpdir, "test1.txt", &keys, &vals, &count, err, sizeof(err)) == 0) {
        printf("Test1 sorted:\n");
        for (size_t i = 0; i < count; i++) {
            printf("  %s=%s\n", keys[i], vals[i]);
            free(keys[i]);
            free(vals[i]);
        }
        free(keys); free(vals);
    } else {
        printf("Test1 error: %s\n", err);
    }

    // Test 2: Whitespace, comments, malformed lines
    const char* t2[] = {"   # comment", " b = x ", "a= y", "invalidline", "=novalue", "keyonly="};
    secure_write_file(tmpdir, "test2.txt", t2, 6, err, sizeof(err));
    keys = NULL; vals = NULL; count = 0;
    if (read_and_sort_key_value_file(tmpdir, "test2.txt", &keys, &vals, &count, err, sizeof(err)) == 0) {
        printf("Test2 sorted:\n");
        for (size_t i = 0; i < count; i++) {
            printf("  %s=%s\n", keys[i], vals[i]);
            free(keys[i]); free(vals[i]);
        }
        free(keys); free(vals);
    } else {
        printf("Test2 error: %s\n", err);
    }

    // Test 3: Duplicate keys
    const char* t3[] = {"dup=2", "dup=1", "alpha=zz", "alpha=aa"};
    secure_write_file(tmpdir, "test3.txt", t3, 4, err, sizeof(err));
    keys = NULL; vals = NULL; count = 0;
    if (read_and_sort_key_value_file(tmpdir, "test3.txt", &keys, &vals, &count, err, sizeof(err)) == 0) {
        printf("Test3 sorted:\n");
        for (size_t i = 0; i < count; i++) {
            printf("  %s=%s\n", keys[i], vals[i]);
            free(keys[i]); free(vals[i]);
        }
        free(keys); free(vals);
    } else {
        printf("Test3 error: %s\n", err);
    }

    // Test 4: Path traversal attempt
    keys = NULL; vals = NULL; count = 0;
    if (read_and_sort_key_value_file(tmpdir, "../outside.txt", &keys, &vals, &count, err, sizeof(err)) == 0) {
        printf("Test4 unexpected success\n");
        for (size_t i = 0; i < count; i++) {
            free(keys[i]); free(vals[i]);
        }
        free(keys); free(vals);
    } else {
        printf("Test4 caught expected error: %s\n", err);
    }

    // Test 5: Absolute path attempt
    const char* t5[] = {"z=last", "m=mid", "a=first"};
    secure_write_file(tmpdir, "test5.txt", t5, 3, err, sizeof(err));
    char abs_path[PATH_MAX];
    snprintf(abs_path, sizeof(abs_path), "%s/%s", tmpdir, "test5.txt");
    keys = NULL; vals = NULL; count = 0;
    if (read_and_sort_key_value_file(tmpdir, abs_path, &keys, &vals, &count, err, sizeof(err)) == 0) {
        printf("Test5 unexpected success\n");
        for (size_t i = 0; i < count; i++) {
            free(keys[i]); free(vals[i]);
        }
        free(keys); free(vals);
    } else {
        printf("Test5 caught expected error: %s\n", err);
    }

    return 0;
}