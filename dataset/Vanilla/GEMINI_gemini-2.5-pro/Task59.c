#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* accum(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t n = strlen(s);
    if (n == 0) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    // Calculate length: sum of 1..n for chars, plus n-1 for hyphens
    size_t char_len = (n * (n + 1)) / 2;
    size_t hyphen_len = n > 0 ? n - 1 : 0;
    size_t total_len = char_len + hyphen_len;

    // Allocate memory (+1 for null terminator)
    char* result = (char*)malloc(total_len + 1);
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    char* p = result;
    for (size_t i = 0; i < n; ++i) {
        *p++ = toupper((unsigned char)s[i]);
        for (size_t j = 0; j < i; ++j) {
            *p++ = tolower((unsigned char)s[i]);
        }
        if (i < n - 1) {
            *p++ = '-';
        }
    }
    *p = '\0';

    return result;
}

int main() {
    const char* tests[] = {"abcd", "RqaEzty", "cwAt", "Z", "Mumbling"};
    for (int i = 0; i < 5; ++i) {
        char* res = accum(tests[i]);
        if (res) {
            printf("Input: \"%s\" -> Output: \"%s\"\n", tests[i], res);
            free(res);
        }
    }
    return 0;
}