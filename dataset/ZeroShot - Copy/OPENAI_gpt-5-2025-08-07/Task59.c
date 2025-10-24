#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* accum(const char* s) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t n = strlen(s);
    if (n == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t total = 0;
    for (size_t i = 0; i < n; ++i) {
        total += (i + 1);
        if (i + 1 < n) total += 1; // hyphen
    }
    char* out = (char*)malloc(total + 1);
    if (!out) return NULL;

    size_t pos = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char uc = (unsigned char)s[i];
        out[pos++] = (char)toupper(uc);
        char lower = (char)tolower(uc);
        for (size_t k = 0; k < i; ++k) {
            out[pos++] = lower;
        }
        if (i + 1 < n) {
            out[pos++] = '-';
        }
    }
    out[pos] = '\0';
    return out;
}

int main(void) {
    const char* tests[] = {"abcd", "RqaEzty", "cwAt", "", "Z"};
    size_t num = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num; ++i) {
        char* res = accum(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
    }
    return 0;
}