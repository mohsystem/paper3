#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool validate_lower_alpha(const char *s) {
    if (s == NULL) {
        return false;
    }
    for (const char *p = s; *p != '\0'; ++p) {
        if (*p < 'a' || *p > 'z') {
            return false;
        }
    }
    return true;
}

char* longest(const char* s1, const char* s2) {
    if (!validate_lower_alpha(s1) || !validate_lower_alpha(s2)) {
        fprintf(stderr, "Error: inputs must contain only 'a' to 'z'.\n");
        return NULL;
    }

    bool seen[26] = { false };
    for (const char* p = s1; p != NULL && *p != '\0'; ++p) {
        seen[(unsigned)(*p - 'a')] = true;
    }
    for (const char* p = s2; p != NULL && *p != '\0'; ++p) {
        seen[(unsigned)(*p - 'a')] = true;
    }

    char* result = (char*)malloc(27);
    if (result == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return NULL;
    }

    size_t idx = 0;
    for (size_t i = 0; i < 26; ++i) {
        if (seen[i]) {
            result[idx++] = (char)('a' + (int)i);
        }
    }
    result[idx] = '\0';
    return result;
}

int main(void) {
    // 5 test cases
    struct TestCase {
        const char* a;
        const char* b;
    } tests[] = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"", ""},
        {"aaaa", "bbb"},
        {"abcxyz", "mnop"}
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char* out = longest(tests[i].a, tests[i].b);
        if (out != NULL) {
            printf("longest(\"%s\", \"%s\") -> \"%s\"\n", tests[i].a, tests[i].b, out);
            free(out);
        } else {
            printf("longest(\"%s\", \"%s\") -> [error]\n", tests[i].a, tests[i].b);
        }
    }

    return 0;
}