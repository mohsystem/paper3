#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Pair {
    char* key;
    char* value;
};

struct PairArray {
    struct Pair* items;
    size_t size;
};

static char* xstrdup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

static void trim_inplace(char* s) {
    if (!s) return;
    char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static int pair_cmp(const void* a, const void* b) {
    const struct Pair* pa = (const struct Pair*)a;
    const struct Pair* pb = (const struct Pair*)b;
    int kc = strcmp(pa->key, pb->key);
    if (kc != 0) return kc;
    return strcmp(pa->value, pb->value);
}

struct PairArray read_and_sort_key_value_file(const char* path) {
    struct PairArray arr;
    arr.items = NULL;
    arr.size = 0;
    size_t cap = 0;

    FILE* f = fopen(path, "r");
    if (!f) return arr;

    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        // remove newline
        size_t len = strlen(buf);
        if (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[len - 1] = '\0';
            len--;
            if (len && buf[len - 1] == '\r') {
                buf[len - 1] = '\0';
                len--;
            }
        }
        char* eq = strchr(buf, '=');
        if (!eq) continue;
        *eq = '\0';
        char* k = buf;
        char* v = eq + 1;

        // make copies to trim independently
        char* kc = xstrdup(k);
        char* vc = xstrdup(v);
        if (!kc || !vc) {
            free(kc); free(vc);
            continue;
        }
        trim_inplace(kc);
        trim_inplace(vc);
        if (kc[0] == '\0' && vc[0] == '\0') {
            free(kc); free(vc);
            continue;
        }

        if (arr.size == cap) {
            size_t ncap = cap == 0 ? 8 : cap * 2;
            struct Pair* nitems = (struct Pair*)realloc(arr.items, ncap * sizeof(struct Pair));
            if (!nitems) {
                free(kc); free(vc);
                break;
            }
            arr.items = nitems;
            cap = ncap;
        }
        arr.items[arr.size].key = kc;
        arr.items[arr.size].value = vc;
        arr.size++;
    }
    fclose(f);

    if (arr.size > 1) {
        qsort(arr.items, arr.size, sizeof(struct Pair), pair_cmp);
    }
    return arr;
}

void free_pair_array(struct PairArray* arr) {
    if (!arr || !arr->items) return;
    for (size_t i = 0; i < arr->size; ++i) {
        free(arr->items[i].key);
        free(arr->items[i].value);
    }
    free(arr->items);
    arr->items = NULL;
    arr->size = 0;
}

static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fputs(content, f);
    fclose(f);
}

static void run_test(const char* title, const char* path) {
    printf("== %s ==\n", title);
    struct PairArray arr = read_and_sort_key_value_file(path);
    for (size_t i = 0; i < arr.size; ++i) {
        printf("%s=%s\n", arr.items[i].key, arr.items[i].value);
    }
    printf("\n");
    free_pair_array(&arr);
}

int main(void) {
    const char* f1 = "test1_kv_c.txt";
    const char* f2 = "test2_kv_c.txt";
    const char* f3 = "test3_kv_c.txt";
    const char* f4 = "test4_kv_c.txt";
    const char* f5 = "test5_kv_c.txt";

    write_file(f1, "b=2\na=1\nc=3\n");
    write_file(f2, "k=3\nk=1\nk=2\n");
    write_file(f3, "  x  =  10  \ninvalid line\ny=5\n=onlyvalue\nonlykey=\n");
    write_file(f4, "10=ten\n2=two\n1=one\n");
    write_file(f5, "a= \n a=0\n b= \n z=last\n   \nnoequalsline\n");

    run_test("Test 1", f1);
    run_test("Test 2", f2);
    run_test("Test 3", f3);
    run_test("Test 4", f4);
    run_test("Test 5", f5);

    return 0;
}