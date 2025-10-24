#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    char** lines;
    size_t count;
} StrArray;

static void free_str_array(StrArray arr) {
    if (arr.lines == NULL) return;
    for (size_t i = 0; i < arr.count; ++i) {
        free(arr.lines[i]);
    }
    free(arr.lines);
}

StrArray number(const char* const* lines, size_t count) {
    StrArray result;
    result.count = 0;
    result.lines = NULL;

    if (lines == NULL || count == 0) {
        return result;
    }

    char** out = (char**)calloc(count, sizeof(char*));
    if (out == NULL) {
        return result;
    }

    for (size_t i = 0; i < count; ++i) {
        const char* s = "";
        if (lines != NULL && lines[i] != NULL) {
            s = lines[i];
        }
        unsigned long long idx = (unsigned long long)(i + 1ULL);

        int needed = snprintf(NULL, 0, "%llu: %s", idx, s);
        if (needed < 0) {
            // Cleanup on error
            for (size_t j = 0; j < i; ++j) {
                free(out[j]);
            }
            free(out);
            result.lines = NULL;
            result.count = 0;
            return result;
        }

        size_t buflen = (size_t)needed + 1U;
        char* buf = (char*)malloc(buflen);
        if (buf == NULL) {
            for (size_t j = 0; j < i; ++j) {
                free(out[j]);
            }
            free(out);
            result.lines = NULL;
            result.count = 0;
            return result;
        }

        int written = snprintf(buf, buflen, "%llu: %s", idx, s);
        if (written < 0 || (size_t)written >= buflen) {
            free(buf);
            for (size_t j = 0; j < i; ++j) {
                free(out[j]);
            }
            free(out);
            result.lines = NULL;
            result.count = 0;
            return result;
        }

        out[i] = buf;
    }

    result.lines = out;
    result.count = count;
    return result;
}

static void print_str_array(const StrArray* arr) {
    if (!arr || !arr->lines) {
        printf("[]\n");
        return;
    }
    printf("[");
    for (size_t i = 0; i < arr->count; ++i) {
        printf("\"%s\"", arr->lines[i] ? arr->lines[i] : "");
        if (i + 1 < arr->count) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    // Test case 1: empty list
    StrArray r1 = number(NULL, 0);
    print_str_array(&r1);
    free_str_array(r1);

    // Test case 2: ["a", "b", "c"]
    const char* t2[] = {"a", "b", "c"};
    StrArray r2 = number(t2, sizeof(t2) / sizeof(t2[0]));
    print_str_array(&r2);
    free_str_array(r2);

    // Test case 3: [NULL, "x", ""]
    const char* t3[] = {NULL, "x", ""};
    StrArray r3 = number(t3, sizeof(t3) / sizeof(t3[0]));
    print_str_array(&r3);
    free_str_array(r3);

    // Test case 4: ["only one"]
    const char* t4[] = {"only one"};
    StrArray r4 = number(t4, sizeof(t4) / sizeof(t4[0]));
    print_str_array(&r4);
    free_str_array(r4);

    // Test case 5: multiple lines
    const char* t5[] = {"line 0", "line 1", "line 2", "line 3", "line 4"};
    StrArray r5 = number(t5, sizeof(t5) / sizeof(t5[0]));
    print_str_array(&r5);
    free_str_array(r5);

    return 0;
}