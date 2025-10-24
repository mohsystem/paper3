#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

/* Step 1-5: Minimal, safe implementation for reading and sorting key-value records in C. */

typedef struct {
    char *key;
    char *value;
} Pair;

typedef struct {
    Pair *items;
    size_t count;
} PairArray;

static char *str_trim(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    size_t newlen = end - start;
    if (start > 0 && newlen > 0) memmove(s, s + start, newlen);
    s[newlen] = '\0';
    return s;
}

static int starts_with(const char *s, const char *prefix) {
    size_t lp = strlen(prefix);
    return strncmp(s, prefix, lp) == 0;
}

static char *read_line_dynamic(FILE *fp) {
    if (!fp) return NULL;
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return NULL;
    int c;
    const size_t MAX_LINE = 2 * 1024 * 1024; // 2MB line cap
    while ((c = fgetc(fp)) != EOF) {
        if (len + 1 >= cap) {
            size_t ncap = cap * 2;
            if (ncap > MAX_LINE) {
                free(buf);
                return NULL;
            }
            char *nb = (char *)realloc(buf, ncap);
            if (!nb) {
                free(buf);
                return NULL;
            }
            buf = nb;
            cap = ncap;
        }
        buf[len++] = (char)c;
        if (c == '\n') break;
    }
    if (len == 0 && c == EOF) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

static int cmp_pairs(const void *a, const void *b) {
    const Pair *pa = (const Pair *)a;
    const Pair *pb = (const Pair *)b;
    if (!pa->key && !pb->key) return 0;
    if (!pa->key) return -1;
    if (!pb->key) return 1;
    return strcmp(pa->key, pb->key);
}

static void free_pair_array(PairArray arr) {
    for (size_t i = 0; i < arr.count; ++i) {
        free(arr.items[i].key);
        free(arr.items[i].value);
    }
    free(arr.items);
}

PairArray read_and_sort_key_values(const char *file_path) {
    const long long MAX_FILE_SIZE = 10LL * 1024LL * 1024LL; // 10MB
    const size_t MAX_KEY_LEN = 4096;
    const size_t MAX_VALUE_LEN = 2000000;

    PairArray out = {NULL, 0};

    if (!file_path) {
        return out;
    }

    struct stat st;
    if (stat(file_path, &st) != 0) {
        return out;
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        return out;
    }
    if (st.st_size > MAX_FILE_SIZE) {
        return out;
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return out;
    }

    // We'll store latest value per key, so manage a dynamic array and replace as needed.
    // For efficiency, we could use a hash table, but to keep simple and safe, we'll
    // store all pairs and later keep only the last occurrence before sorting.
    Pair *pairs = NULL;
    size_t count = 0, cap = 0;
    long long totalValueBytes = 0;

    char *line;
    while ((line = read_line_dynamic(fp)) != NULL) {
        // replace newline
        size_t ll = strlen(line);
        if (ll > 0 && (line[ll-1] == '\n' || line[ll-1] == '\r')) {
            line[ll-1] = '\0';
            if (ll > 1 && line[ll-2] == '\r') line[ll-2] = '\0';
        }
        str_trim(line);
        if (line[0] == '\0' || starts_with(line, "#") || starts_with(line, "//")) {
            free(line);
            continue;
        }

        char *eq = strchr(line, '=');
        char *col = strchr(line, ':');
        char *sep = NULL;
        if (!eq && !col) {
            free(line);
            continue;
        } else if (!eq) sep = col;
        else if (!col) sep = eq;
        else sep = (eq < col) ? eq : col;

        // split
        *sep = '\0';
        char *key = str_trim(line);
        char *value = str_trim(sep + 1);

        if (key[0] == '\0') {
            free(line);
            continue;
        }
        if (strlen(key) > MAX_KEY_LEN || strlen(value) > MAX_VALUE_LEN) {
            free(line);
            continue;
        }
        totalValueBytes += (long long)strlen(value);
        if (totalValueBytes > MAX_FILE_SIZE) {
            free(line);
            break;
        }

        // Copy strings to independent allocations
        char *kcopy = (char *)malloc(strlen(key) + 1);
        char *vcopy = (char *)malloc(strlen(value) + 1);
        if (!kcopy || !vcopy) {
            free(kcopy);
            free(vcopy);
            free(line);
            break;
        }
        strcpy(kcopy, key);
        strcpy(vcopy, value);

        if (count == cap) {
            size_t ncap = cap == 0 ? 16 : cap * 2;
            Pair *np = (Pair *)realloc(pairs, ncap * sizeof(Pair));
            if (!np) {
                free(kcopy);
                free(vcopy);
                free(line);
                break;
            }
            pairs = np;
            cap = ncap;
        }
        pairs[count].key = kcopy;
        pairs[count].value = vcopy;
        count++;

        free(line);
    }
    fclose(fp);

    if (count == 0) {
        free(pairs);
        return out;
    }

    // Keep latest occurrence per key: build a map-like structure with linear search (simple).
    // First, sort by key then stable-keep last; but simpler: traverse and replace older.
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ) {
            if (strcmp(pairs[i].key, pairs[j].key) == 0) {
                // replace i with j (keep latest), remove j by swapping with last
                free(pairs[i].value);
                pairs[i].value = strdup(pairs[j].value);
                // remove pairs[j]
                free(pairs[j].key);
                free(pairs[j].value);
                if (j != count - 1) {
                    pairs[j] = pairs[count - 1];
                }
                count--;
            } else {
                j++;
            }
        }
    }

    qsort(pairs, count, sizeof(Pair), cmp_pairs);

    out.items = pairs;
    out.count = count;
    return out;
}

static void write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    fwrite(content, 1, strlen(content), fp);
    fclose(fp);
}

static void run_test(const char *title, const char *filename, const char *content) {
    write_file(filename, content);
    PairArray arr = read_and_sort_key_values(filename);
    printf("=== %s (%s) ===\n", title, filename);
    for (size_t i = 0; i < arr.count; ++i) {
        printf("%s=%s\n", arr.items[i].key, arr.items[i].value);
    }
    free_pair_array(arr);
}

int main(void) {
    run_test("Test 1 - Basic", "c_test_kv_1.txt",
             "banana=yellow\napple=red\ncarrot=orange\n");
    run_test("Test 2 - Unsorted with spaces", "c_test_kv_2.txt",
             "  zeta = 26 \nalpha=1\nbeta = 2\n  gamma= 3\n");
    run_test("Test 3 - Duplicates", "c_test_kv_3.txt",
             "a=1\na=2\nb=3\nA=upper\n");
    run_test("Test 4 - Mixed separators and comments", "c_test_kv_4.txt",
             "# Comment line\n// Another comment\nhost: localhost\nport=8080\n user : admin \npassword = secret\n");
    run_test("Test 5 - Special characters", "c_test_kv_5.txt",
             "path=/usr/local/bin\nkey.with.dots=value.with.dots\n spaced key = spaced value \ncolon:used\n");
    return 0;
}