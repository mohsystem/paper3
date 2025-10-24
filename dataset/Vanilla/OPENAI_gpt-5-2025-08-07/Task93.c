#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* key;
    char* value;
} Pair;

static char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static void trim_inplace(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && isspace((unsigned char)s[i])) i++;
    size_t j = len;
    while (j > i && isspace((unsigned char)s[j - 1])) j--;
    size_t newlen = j > i ? (j - i) : 0;
    if (i > 0 && newlen > 0) memmove(s, s + i, newlen);
    s[newlen] = '\0';
}

static int find_first_sep(const char* s) {
    for (int i = 0; s[i]; ++i) {
        if (s[i] == '=' || s[i] == ':') return i;
    }
    return -1;
}

static int cmp_pairs(const void* a, const void* b) {
    const Pair* pa = (const Pair*)a;
    const Pair* pb = (const Pair*)b;
    int c = strcmp(pa->key, pb->key);
    if (c != 0) return c;
    return strcmp(pa->value, pb->value);
}

char* sortKeyValueFile(const char* filePath) {
    FILE* f = fopen(filePath, "r");
    if (!f) return strdup_safe("");
    Pair* arr = NULL;
    size_t size = 0, cap = 0;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        // Remove trailing newline
        size_t n = strlen(buf);
        if (n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = '\0';
        if (n && buf[n - 1] == '\r') buf[--n] = '\0';
        char* line = strdup_safe(buf);
        if (!line) continue;
        trim_inplace(line);
        if (line[0] == '\0' || line[0] == '#') { free(line); continue; }
        int idx = find_first_sep(line);
        if (idx <= 0) { free(line); continue; }
        line[idx] = '\0';
        char* key = strdup_safe(line);
        char* value = strdup_safe(line + idx + 1);
        trim_inplace(key);
        trim_inplace(value);
        if (key[0] == '\0') { free(key); free(value); free(line); continue; }
        if (size == cap) {
            cap = cap ? cap * 2 : 16;
            Pair* tmp = (Pair*)realloc(arr, cap * sizeof(Pair));
            if (!tmp) { free(key); free(value); free(line); break; }
            arr = tmp;
        }
        arr[size].key = key;
        arr[size].value = value;
        size++;
        free(line);
    }
    fclose(f);
    if (size == 0) {
        free(arr);
        return strdup_safe("");
    }
    qsort(arr, size, sizeof(Pair), cmp_pairs);
    size_t total = 0;
    for (size_t i = 0; i < size; ++i) {
        total += strlen(arr[i].key) + 1 + strlen(arr[i].value) + 1; // '=' and '\n'
    }
    char* out = (char*)malloc(total + 1);
    if (!out) {
        for (size_t i = 0; i < size; ++i) { free(arr[i].key); free(arr[i].value); }
        free(arr);
        return strdup_safe("");
    }
    char* p = out;
    for (size_t i = 0; i < size; ++i) {
        size_t klen = strlen(arr[i].key);
        size_t vlen = strlen(arr[i].value);
        memcpy(p, arr[i].key, klen); p += klen;
        *p++ = '=';
        memcpy(p, arr[i].value, vlen); p += vlen;
        *p++ = '\n';
    }
    *p = '\0';
    for (size_t i = 0; i < size; ++i) { free(arr[i].key); free(arr[i].value); }
    free(arr);
    return out;
}

static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

int main(void) {
    const char* f1 = "task93_c_test1.txt";
    const char* f2 = "task93_c_test2.txt";
    const char* f3 = "task93_c_test3.txt";
    const char* f4 = "task93_c_test4.txt";
    const char* f5 = "task93_c_test5.txt";

    write_file(f1, "b=2\na=1\nc=3\n");
    write_file(f2, "orange:10\napple:20\nbanana:15\n");
    write_file(f3, " # comment\n z : last \n\n a : first \n middle=between \n");
    write_file(f4, "dup=2\ndup=1\nsame:0\nsame:5\n");
    write_file(f5, "invalidline\n k=v\n x : y \n #comment \n :missingkey? \n =missingkey2 \n");

    char* r1 = sortKeyValueFile(f1);
    char* r2 = sortKeyValueFile(f2);
    char* r3 = sortKeyValueFile(f3);
    char* r4 = sortKeyValueFile(f4);
    char* r5 = sortKeyValueFile(f5);

    printf("Test1:\n%s", r1);
    printf("Test2:\n%s", r2);
    printf("Test3:\n%s", r3);
    printf("Test4:\n%s", r4);
    printf("Test5:\n%s", r5);

    free(r1); free(r2); free(r3); free(r4); free(r5);
    return 0;
}