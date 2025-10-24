#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_REL_PATH_LENGTH 4096
#define MAX_LINE_LENGTH 8192

typedef struct {
    char* key;
    char* value;
} kv_pair;

typedef struct {
    kv_pair* items;
    size_t size;
    size_t capacity;
} kv_vector;

static void kv_vector_init(kv_vector* v) {
    v->items = NULL;
    v->size = 0;
    v->capacity = 0;
}
static void kv_vector_free(kv_vector* v) {
    if (!v) return;
    for (size_t i = 0; i < v->size; ++i) {
        free(v->items[i].key);
        free(v->items[i].value);
    }
    free(v->items);
    v->items = NULL;
    v->size = v->capacity = 0;
}
static int kv_vector_push(kv_vector* v, const char* key, const char* value) {
    if (v->size == v->capacity) {
        size_t newcap = v->capacity == 0 ? 8 : v->capacity * 2;
        kv_pair* n = (kv_pair*)realloc(v->items, newcap * sizeof(kv_pair));
        if (!n) return -1;
        v->items = n;
        v->capacity = newcap;
    }
    v->items[v->size].key = strdup(key);
    v->items[v->size].value = strdup(value);
    if (!v->items[v->size].key || !v->items[v->size].value) return -1;
    v->size += 1;
    return 0;
}

static char* str_trim(char* s) {
    if (!s) return s;
    size_t len = strlen(s);
    size_t i = 0, j = len;
    while (i < len && isspace((unsigned char)s[i])) i++;
    while (j > i && isspace((unsigned char)s[j - 1])) j--;
    if (i > 0) memmove(s, s + i, j - i);
    s[j - i] = '\0';
    return s;
}

static int cmp_pairs(const void* a, const void* b) {
    const kv_pair* pa = (const kv_pair*)a;
    const kv_pair* pb = (const kv_pair*)b;
    int r = strcmp(pa->key, pb->key);
    if (r != 0) return r;
    return strcmp(pa->value, pb->value);
}

static int is_symlink_path(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) return 0; // if not exists, treat as not symlink here
    return S_ISLNK(st.st_mode) ? 1 : 0;
}

static int ensure_within_base(const char* base_real, const char* target_real) {
    size_t br = strlen(base_real);
    if (strncmp(base_real, target_real, br) != 0) return 0;
    // Ensure boundary at separator
    if (target_real[br] == '\0' || target_real[br] == '/' ) return 1;
    return 0;
}

static int has_null_byte(const char* s) {
    // In C strings, presence of '\0' indicates end; for validation we just ensure pointer not NULL
    return s == NULL;
}

static int reject_path_traversal(const char* rel) {
    // Reject ".." components
    const char* p = rel;
    while (*p) {
        if ((p[0] == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\0')) ||
            (p[0] == '/' && p[1] == '.' && p[2] == '.' && (p[3] == '/' || p[3] == '\0'))) {
            return -1;
        }
        p++;
    }
    return 0;
}

static int join_paths(const char* base, const char* rel, char* out, size_t outsz) {
    size_t bl = strlen(base);
    size_t rl = strlen(rel);
    if (bl + 1 + rl + 1 > outsz) return -1;
    strcpy(out, base);
    if (bl > 0 && out[bl - 1] != '/') {
        out[bl] = '/';
        out[bl + 1] = '\0';
    }
    strcat(out, rel);
    return 0;
}

static int resolve_safe_path(const char* base_dir, const char* relative, char* out, size_t outsz) {
    if (!base_dir || !relative) return -1;
    struct stat st_base;
    if (lstat(base_dir, &st_base) != 0 || !S_ISDIR(st_base.st_mode)) {
        return -1;
    }
    if (S_ISLNK(st_base.st_mode)) {
        return -1;
    }
    if (strlen(relative) == 0 || strlen(relative) > MAX_REL_PATH_LENGTH) {
        return -1;
    }
    if (relative[0] == '/') {
        return -1;
    }
    if (has_null_byte(relative)) {
        return -1;
    }
    if (reject_path_traversal(relative) != 0) {
        return -1;
    }
    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        return -1;
    }
    // Build combined path
    char combined[PATH_MAX];
    if (join_paths(base_real, relative, combined, sizeof(combined)) != 0) {
        return -1;
    }
    // Walk each component and ensure no symlink
    char pathbuf[PATH_MAX];
    strncpy(pathbuf, combined, sizeof(pathbuf) - 1);
    pathbuf[sizeof(pathbuf) - 1] = '\0';
    char temp[PATH_MAX];
    temp[0] = '\0';
    const char* delim = "/";
    char* saveptr = NULL;
    char* token = strtok_r(pathbuf, delim, &saveptr);
    strcpy(temp, base_real);
    while (token) {
        size_t tl = strlen(temp);
        if (tl + 1 + strlen(token) + 1 >= sizeof(temp)) return -1;
        if (tl == 0 || temp[tl - 1] != '/') strcat(temp, "/");
        strcat(temp, token);
        if (is_symlink_path(temp)) {
            return -1;
        }
        token = strtok_r(NULL, delim, &saveptr);
    }
    // Resolve real path to ensure containment
    char target_real[PATH_MAX];
    if (!realpath(combined, target_real)) {
        // If file doesn't exist yet, fail
        return -1;
    }
    if (!ensure_within_base(base_real, target_real)) {
        return -1;
    }
    // Must be regular file and not symlink
    struct stat st_target;
    if (lstat(target_real, &st_target) != 0) return -1;
    if (S_ISLNK(st_target.st_mode) || !S_ISREG(st_target.st_mode)) return -1;

    if (strlen(target_real) + 1 > outsz) return -1;
    strcpy(out, target_real);
    return 0;
}

static int parse_file_fd(int fd, char delimiter, kv_vector* out_vec) {
    if (delimiter == '\n' || delimiter == '\r') return -1;
    char buffer[4096];
    size_t cap = 8192;
    char* linebuf = (char*)malloc(cap);
    if (!linebuf) return -1;
    size_t linelen = 0;

    for (;;) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n < 0) {
            free(linebuf);
            return -1;
        }
        if (n == 0) {
            // flush last line
            if (linelen > 0) {
                if (linelen > MAX_LINE_LENGTH) { free(linebuf); return -1; }
                linebuf[linelen] = '\0';
                // process line
                char* line = linebuf;
                size_t llen = strlen(line);
                if (llen > 0 && line[llen - 1] == '\r') line[llen - 1] = '\0';
                // Remove BOM if first line
                static int first_line = 1;
                if (first_line && (unsigned char)line[0] == 0xEF &&
                    (unsigned char)line[1] == 0xBB &&
                    (unsigned char)line[2] == 0xBF) {
                    memmove(line, line + 3, strlen(line + 3) + 1);
                }
                first_line = 0;
                char* t = str_trim(line);
                if (t[0] != '\0' && t[0] != '#') {
                    char* pos = strchr(t, delimiter);
                    if (pos) {
                        *pos = '\0';
                        char* key = str_trim(t);
                        char* value = str_trim(pos + 1);
                        if (key[0] != '\0') {
                            if (kv_vector_push(out_vec, key, value) != 0) {
                                free(linebuf);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        for (ssize_t i = 0; i < n; ++i) {
            char c = buffer[i];
            if (linelen + 1 >= cap) {
                size_t ncap = cap * 2;
                char* nb = (char*)realloc(linebuf, ncap);
                if (!nb) { free(linebuf); return -1; }
                linebuf = nb;
                cap = ncap;
            }
            if (c == '\n') {
                linebuf[linelen] = '\0';
                // process line
                char* line = linebuf;
                size_t llen = strlen(line);
                if (llen > MAX_LINE_LENGTH) { free(linebuf); return -1; }
                if (llen > 0 && line[llen - 1] == '\r') line[llen - 1] = '\0';
                static int first_line2 = 1;
                if (first_line2 && (unsigned char)line[0] == 0xEF &&
                    (unsigned char)line[1] == 0xBB &&
                    (unsigned char)line[2] == 0xBF) {
                    memmove(line, line + 3, strlen(line + 3) + 1);
                }
                first_line2 = 0;
                char* t = str_trim(line);
                if (t[0] != '\0' && t[0] != '#') {
                    char* pos = strchr(t, delimiter);
                    if (pos) {
                        *pos = '\0';
                        char* key = str_trim(t);
                        char* value = str_trim(pos + 1);
                        if (key[0] != '\0') {
                            if (kv_vector_push(out_vec, key, value) != 0) {
                                free(linebuf);
                                return -1;
                            }
                        }
                    }
                }
                linelen = 0;
            } else {
                linebuf[linelen++] = c;
                if (linelen > MAX_LINE_LENGTH) {
                    free(linebuf);
                    return -1;
                }
            }
        }
    }
    free(linebuf);
    return 0;
}

static char* read_and_sort_kv_file(const char* base_dir, const char* relative_path, char delimiter) {
    if (!base_dir || !relative_path) return NULL;
    char safe_path[PATH_MAX];
    if (resolve_safe_path(base_dir, relative_path, safe_path, sizeof(safe_path)) != 0) {
        return NULL;
    }
    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
    int fd = open(safe_path, flags);
    if (fd < 0) {
        return NULL;
    }
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        return NULL;
    }

    kv_vector vec;
    kv_vector_init(&vec);
    if (parse_file_fd(fd, delimiter, &vec) != 0) {
        close(fd);
        kv_vector_free(&vec);
        return NULL;
    }
    close(fd);

    qsort(vec.items, vec.size, sizeof(kv_pair), cmp_pairs);

    // Build output string "key=value\n" per entry
    size_t total = 1; // for trailing '\0'
    for (size_t i = 0; i < vec.size; ++i) {
        total += strlen(vec.items[i].key) + 1 + strlen(vec.items[i].value) + 1;
    }
    char* out = (char*)malloc(total);
    if (!out) {
        kv_vector_free(&vec);
        return NULL;
    }
    out[0] = '\0';
    for (size_t i = 0; i < vec.size; ++i) {
        strcat(out, vec.items[i].key);
        strcat(out, "=");
        strcat(out, vec.items[i].value);
        strcat(out, "\n");
    }
    kv_vector_free(&vec);
    return out;
}

static int write_file(const char* base_dir, const char* rel, const char* content) {
    char path[PATH_MAX];
    // Simple join for tests
    if (snprintf(path, sizeof(path), "%s/%s", base_dir, rel) >= (int)sizeof(path)) return -1;
    // Create directories if needed
    char dirbuf[PATH_MAX];
    strncpy(dirbuf, path, sizeof(dirbuf) - 1);
    dirbuf[sizeof(dirbuf) - 1] = '\0';
    for (char* p = dirbuf + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            mkdir(dirbuf, 0700);
            *p = '/';
        }
    }
    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd < 0) return -1;
    size_t len = strlen(content);
    ssize_t w = write(fd, content, len);
    close(fd);
    return (w == (ssize_t)len) ? 0 : -1;
}

int main(void) {
    char base[PATH_MAX];
    // Create a temp directory for tests
    snprintf(base, sizeof(base), "/tmp/kv_safe_base_c_%ld", (long)time(NULL));
    mkdir(base, 0700);

    write_file(base, "t1.txt", "apple=3\nbanana=1\ncherry=2\n");
    write_file(base, "t2.txt", "# comment\norange = 10\n banana = 5\nbanana = 2\n");
    write_file(base, "dir/sub/t3.txt", "z=last\n a=first\nm=mid\n");
    write_file(base, "t4.txt", "x=a=b=c\nn=:emptyBefore\n:badline\n");
    write_file(base, "t5.txt", "key with spaces = value with spaces\nalpha= \n gamma=3\n");

    char* r1 = read_and_sort_kv_file(base, "t1.txt", '=');
    char* r2 = read_and_sort_kv_file(base, "t2.txt", '=');
    char* r3 = read_and_sort_kv_file(base, "dir/sub/t3.txt", '=');
    char* r4 = read_and_sort_kv_file(base, "t4.txt", '=');
    char* r5 = read_and_sort_kv_file(base, "t5.txt", '=');

    printf("Test 1:\n%s---\n", r1 ? r1 : "(error)\n");
    printf("Test 2:\n%s---\n", r2 ? r2 : "(error)\n");
    printf("Test 3:\n%s---\n", r3 ? r3 : "(error)\n");
    printf("Test 4:\n%s---\n", r4 ? r4 : "(error)\n");
    printf("Test 5:\n%s---\n", r5 ? r5 : "(error)\n");

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);

    return 0;
}