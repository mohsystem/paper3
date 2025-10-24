/* Chain-of-Through in comments: secure parsing, trimming, bounded resources, sorting, and testing. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_RECORDS 1000000
#define LINE_BUF 65536

typedef struct {
    char* key;
    char* value;
} Pair;

typedef struct {
    Pair* items;
    size_t count;
} KVList;

static char* str_dup_safe(const char* s, size_t len) {
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

static void free_kvlist(KVList* list) {
    if (!list || !list->items) return;
    for (size_t i = 0; i < list->count; ++i) {
        free(list->items[i].key);
        free(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

static void rstrip_newline(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

static void trim_range(const char* s, size_t* start, size_t* end) {
    while (*start < *end && (s[*start] == ' ' || s[*start] == '\t' || s[*start] == '\r' || s[*start] == '\n'))
        (*start)++;
    while (*end > *start && (s[*end - 1] == ' ' || s[*end - 1] == '\t' || s[*end - 1] == '\r' || s[*end - 1] == '\n'))
        (*end)--;
}

static int is_comment_or_empty(const char* s) {
    // Check if after trimming leading whitespace, line starts with '#', or empty
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') i++;
    if (s[i] == '\0') return 1;
    if (s[i] == '#') return 1;
    return 0;
}

static int pair_cmp(const void* a, const void* b) {
    const Pair* pa = (const Pair*)a;
    const Pair* pb = (const Pair*)b;
    int c = strcmp(pa->key, pb->key);
    if (c != 0) return c;
    return strcmp(pa->value, pb->value);
}

KVList read_and_sort_records(const char* path) {
    KVList out = {0};
    FILE* f = fopen(path, "rb");
    if (!f) {
        return out; // return empty on failure
    }
    size_t cap = 64;
    out.items = (Pair*)calloc(cap, sizeof(Pair));
    if (!out.items) {
        fclose(f);
        return out;
    }

    char buf[LINE_BUF];
    while (fgets(buf, sizeof(buf), f)) {
        // If line longer than buffer, discard remainder to avoid overly long lines
        if (strchr(buf, '\n') == NULL) {
            int ch;
            // discard until newline or EOF
            while ((ch = fgetc(f)) != EOF) {
                if (ch == '\n') break;
            }
        }
        rstrip_newline(buf);
        if (is_comment_or_empty(buf)) continue;

        // Find first '='
        char* eq = strchr(buf, '=');
        if (!eq) continue;

        // Determine key range
        size_t start = 0;
        size_t end = (size_t)(eq - buf);
        trim_range(buf, &start, &end);
        if (end <= start) continue; // empty key after trim

        // Determine value range
        size_t vstart = (size_t)(eq - buf + 1);
        size_t vend = strlen(buf);
        trim_range(buf, &vstart, &vend);

        // Allocate key and value
        size_t klen = end - start;
        size_t vlen = vend > vstart ? (vend - vstart) : 0;
        char* key = str_dup_safe(buf + start, klen);
        char* value = str_dup_safe(buf + vstart, vlen);
        if (!key || !value) {
            free(key);
            free(value);
            free_kvlist(&out);
            fclose(f);
            KVList empty = {0};
            return empty;
        }

        if (out.count == cap) {
            if (cap > MAX_RECORDS / 2) {
                // Prevent overflow and resource exhaustion
                free(key);
                free(value);
                free_kvlist(&out);
                fclose(f);
                KVList empty = {0};
                return empty;
            }
            cap *= 2;
            Pair* tmp = (Pair*)realloc(out.items, cap * sizeof(Pair));
            if (!tmp) {
                free(key);
                free(value);
                free_kvlist(&out);
                fclose(f);
                KVList empty = {0};
                return empty;
            }
            out.items = tmp;
        }
        out.items[out.count].key = key;
        out.items[out.count].value = value;
        out.count++;

        if (out.count > MAX_RECORDS) {
            free_kvlist(&out);
            fclose(f);
            KVList empty = {0};
            return empty;
        }
    }
    fclose(f);

    if (out.count > 1) {
        qsort(out.items, out.count, sizeof(Pair), pair_cmp);
    }
    return out;
}

static void print_records(const KVList* list) {
    printf("[");
    for (size_t i = 0; i < list->count; ++i) {
        printf("%s=%s", list->items[i].key, list->items[i].value);
        if (i + 1 < list->count) printf(", ");
    }
    printf("]\n");
}

static int write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t len = strlen(content);
    if (len > 0) {
        if (fwrite(content, 1, len, f) != len) {
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

int main(void) {
    const char* files[5] = {
        "task94_c_test_1.txt",
        "task94_c_test_2.txt",
        "task94_c_test_3.txt",
        "task94_c_test_4.txt",
        "task94_c_test_5.txt"
    };
    const char* contents[5] = {
        "b=2\na=1\nc=3\n",
        "  key2 =  z \n# comment\n\nkey1= a \n  # another comment\n",
        "k=2\nk=1\nk=3\n",
        "noequal\n=onlyvalue\nonlykey=\n mid = val = extra \n",
        "äpple=1\nbanana=2\nÁlpha=0\n"
    };

    for (int i = 0; i < 5; ++i) {
        if (write_file(files[i], contents[i]) != 0) {
            fprintf(stderr, "Failed to write test file %s\n", files[i]);
            return 1;
        }
    }

    for (int i = 0; i < 5; ++i) {
        KVList list = read_and_sort_records(files[i]);
        printf("Test %d result: ", i + 1);
        print_records(&list);
        free_kvlist(&list);
    }

    for (int i = 0; i < 5; ++i) {
        remove(files[i]);
    }

    return 0;
}