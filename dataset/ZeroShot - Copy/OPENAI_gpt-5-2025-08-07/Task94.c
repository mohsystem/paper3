#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __unix__
#include <sys/stat.h>
#endif

#define MAX_LINE_LEN 1048576

typedef struct {
    char *key;
    char *value;
} Pair;

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *d = (char *)malloc(len + 1);
    if (!d) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

static char *trim_inplace(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
    return s;
}

static int is_comment_or_empty(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return (*s == '\0' || *s == '#' || *s == ';');
}

// Read a line of arbitrary length safely. Returns malloc'd string (without trailing '
') or NULL on EOF/error.
// Skips returning lines longer than MAX_LINE_LEN to avoid memory issues (drains until newline).
static char *read_line(FILE *fp) {
    if (!fp) return NULL;
    size_t cap = 0;
    size_t len = 0;
    char *buf = NULL;
    int c;
    int too_long = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (!too_long) {
            if (len + 1 >= cap) {
                size_t newcap = cap ? cap * 2 : 128;
                if (newcap > MAX_LINE_LEN + 2) {
                    too_long = 1;
                } else {
                    char *nb = (char *)realloc(buf, newcap);
                    if (!nb) {
                        free(buf);
                        // drain the rest of the line
                        while (c != '\n' && (c = fgetc(fp)) != EOF) {}
                        return NULL;
                    }
                    buf = nb;
                    cap = newcap;
                }
            }
            if (!too_long) {
                buf[len++] = (char)c;
            }
        }
        if (c == '\n') break;
    }
    if (len == 0 && c == EOF) {
        free(buf);
        return NULL;
    }
    if (too_long) {
        // drain until newline already done in loop; skip this line
        free(buf);
        return safe_strdup("");
    }
    if (buf == NULL) {
        buf = (char *)malloc(1);
        if (!buf) return NULL;
    }
    buf[len] = '\0';
    // Strip trailing '\r' and '\n'
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }
    return buf;
}

static int pair_cmp(const void *a, const void *b) {
    const Pair *pa = (const Pair *)a;
    const Pair *pb = (const Pair *)b;
    int k = strcmp(pa->key ? pa->key : "", pb->key ? pb->key : "");
    if (k != 0) return k;
    return strcmp(pa->value ? pa->value : "", pb->value ? pb->value : "");
}

// Returns an array of strings "key=value" and sets out_count. Caller must free each string and the array.
// On error, returns NULL and out_count is set to 0.
char **read_and_sort_key_value_file(const char *file_path, size_t *out_count) {
    if (!out_count) return NULL;
    *out_count = 0;
    if (!file_path || file_path[0] == '\0') {
        return NULL;
    }
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return NULL;
    }
    Pair *pairs = NULL;
    size_t pairs_count = 0, pairs_cap = 0;

    for (;;) {
        char *line = read_line(fp);
        if (!line) break;
        // Skip empty holder for too long lines
        if (line[0] == '\0') {
            free(line);
            continue;
        }
        char *t = trim_inplace(line);
        if (is_comment_or_empty(t)) {
            free(line);
            continue;
        }
        char *eq = strchr(t, '=');
        if (!eq || eq == t) {
            free(line);
            continue;
        }
        *eq = '\0';
        char *key = trim_inplace(t);
        char *value = trim_inplace(eq + 1);
        if (!key || key[0] == '\0') {
            free(line);
            continue;
        }
        char *kdup = safe_strdup(key);
        char *vdup = safe_strdup(value ? value : "");
        free(line);
        if (!kdup || !vdup) {
            free(kdup);
            free(vdup);
            // cleanup
            for (size_t i = 0; i < pairs_count; ++i) {
                free(pairs[i].key);
                free(pairs[i].value);
            }
            free(pairs);
            fclose(fp);
            return NULL;
        }
        if (pairs_count == pairs_cap) {
            size_t newcap = pairs_cap ? pairs_cap * 2 : 16;
            Pair *np = (Pair *)realloc(pairs, newcap * sizeof(Pair));
            if (!np) {
                free(kdup);
                free(vdup);
                for (size_t i = 0; i < pairs_count; ++i) {
                    free(pairs[i].key);
                    free(pairs[i].value);
                }
                free(pairs);
                fclose(fp);
                return NULL;
            }
            pairs = np;
            pairs_cap = newcap;
        }
        pairs[pairs_count].key = kdup;
        pairs[pairs_count].value = vdup;
        pairs_count++;
    }
    fclose(fp);

    qsort(pairs, pairs_count, sizeof(Pair), pair_cmp);

    char **result = NULL;
    if (pairs_count > 0) {
        result = (char **)calloc(pairs_count, sizeof(char *));
        if (!result) {
            for (size_t i = 0; i < pairs_count; ++i) {
                free(pairs[i].key);
                free(pairs[i].value);
            }
            free(pairs);
            return NULL;
        }
        for (size_t i = 0; i < pairs_count; ++i) {
            size_t lk = strlen(pairs[i].key);
            size_t lv = strlen(pairs[i].value);
            char *s = (char *)malloc(lk + 1 + lv + 1);
            if (!s) {
                // cleanup partial
                for (size_t j = 0; j < i; ++j) free(result[j]);
                free(result);
                for (size_t j = 0; j < pairs_count; ++j) {
                    free(pairs[j].key);
                    free(pairs[j].value);
                }
                free(pairs);
                return NULL;
            }
            memcpy(s, pairs[i].key, lk);
            s[lk] = '=';
            memcpy(s + lk + 1, pairs[i].value, lv);
            s[lk + 1 + lv] = '\0';
            result[i] = s;
        }
    }
    for (size_t i = 0; i < pairs_count; ++i) {
        free(pairs[i].key);
        free(pairs[i].value);
    }
    free(pairs);
    *out_count = pairs_count;
    return result;
}

// Helper to create a secure temp file and write content, returns malloc'd path string.
static char *create_temp_file_with_content(const char *content) {
#ifdef __unix__
    char tmpl[] = "/tmp/task94_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        return NULL;
    }
    size_t len = content ? strlen(content) : 0;
    const char *p = content;
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w < 0) {
            int err = errno;
            close(fd);
            unlink(tmpl);
            errno = err;
            return NULL;
        }
        p += (size_t)w;
        len -= (size_t)w;
    }
    if (close(fd) < 0) {
        unlink(tmpl);
        return NULL;
    }
    // Return a strdup of the path, since tmpl is on stack
    return safe_strdup(tmpl);
#else
    // Fallback (non-POSIX): attempt to use tmpnam and fopen with exclusive create if available
    char *name = tmpnam(NULL);
    if (!name) return NULL;
    FILE *f = fopen(name, "wbx");
    if (!f) return NULL;
    if (content && *content) {
        if (fwrite(content, 1, strlen(content), f) != strlen(content)) {
            fclose(f);
            remove(name);
            return NULL;
        }
    }
    fclose(f);
    return safe_strdup(name);
#endif
}

int main(void) {
    const char *c1 =
        "# Comment line\n"
        "a=1\n"
        "b= 2\n"
        " c =3 \n"
        "d=4\n"
        "invalidline\n"
        "=novalue\n"
        "e=\n"
        ";comment\n"
        "f = value with spaces  \n";
    const char *c2 =
        "z = last\n"
        "a = first\n"
        "m = middle\n"
        "a = duplicate\n"
        "x=42\n"
        "y=100\n";
    const char *c3 =
        " key = value=with=equals\n"
        " spaced key = spaced value \n"
        "# comment\n"
        "emptykey= \n"
        " = bad\n"
        "onlykey=\n";
    const char *c4 =
        "café=au lait\n"
        "naïve=façade\n"
        "日本=語\n"
        "ключ=значение\n";
    const char *c5 =
        "delta=4\r\n"
        "alpha=1\r\n"
        "charlie=3\r\n"
        "bravo=2\r\n"
        "alpha=0\r\n";

    char *p1 = create_temp_file_with_content(c1);
    char *p2 = create_temp_file_with_content(c2);
    char *p3 = create_temp_file_with_content(c3);
    char *p4 = create_temp_file_with_content(c4);
    char *p5 = create_temp_file_with_content(c5);

    if (!p1 || !p2 || !p3 || !p4 || !p5) {
        fprintf(stderr, "Failed to create temp files\n");
        free(p1); free(p2); free(p3); free(p4); free(p5);
        return 1;
    }

    size_t n = 0;
    char **res = NULL;

    res = read_and_sort_key_value_file(p1, &n);
    printf("Test 1\n");
    if (res) { for (size_t i = 0; i < n; ++i) { printf("%s\n", res[i]); free(res[i]); } free(res); }
    printf("----\n");

    res = read_and_sort_key_value_file(p2, &n);
    printf("Test 2\n");
    if (res) { for (size_t i = 0; i < n; ++i) { printf("%s\n", res[i]); free(res[i]); } free(res); }
    printf("----\n");

    res = read_and_sort_key_value_file(p3, &n);
    printf("Test 3\n");
    if (res) { for (size_t i = 0; i < n; ++i) { printf("%s\n", res[i]); free(res[i]); } free(res); }
    printf("----\n");

    res = read_and_sort_key_value_file(p4, &n);
    printf("Test 4\n");
    if (res) { for (size_t i = 0; i < n; ++i) { printf("%s\n", res[i]); free(res[i]); } free(res); }
    printf("----\n");

    res = read_and_sort_key_value_file(p5, &n);
    printf("Test 5\n");
    if (res) { for (size_t i = 0; i < n; ++i) { printf("%s\n", res[i]); free(res[i]); } free(res); }
    printf("----\n");

#ifdef __unix__
    unlink(p1); unlink(p2); unlink(p3); unlink(p4); unlink(p5);
#else
    remove(p1); remove(p2); remove(p3); remove(p4); remove(p5);
#endif
    free(p1); free(p2); free(p3); free(p4); free(p5);
    return 0;
}