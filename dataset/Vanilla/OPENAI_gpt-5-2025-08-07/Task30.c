#include <stdio.h>
#include <stdlib.h>

char* longest(const char* s1, const char* s2) {
    int present[26] = {0};
    if (s1) {
        for (const char* p = s1; *p; ++p) {
            char c = *p;
            if (c >= 'a' && c <= 'z') present[c - 'a'] = 1;
        }
    }
    if (s2) {
        for (const char* p = s2; *p; ++p) {
            char c = *p;
            if (c >= 'a' && c <= 'z') present[c - 'a'] = 1;
        }
    }
    char* res = (char*)malloc(27);
    if (!res) return NULL;
    int pos = 0;
    for (int i = 0; i < 26; ++i) {
        if (present[i]) res[pos++] = (char)('a' + i);
    }
    res[pos] = '\0';
    return res;
}

int main() {
    const char* tests[5][2] = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"", ""},
        {"aaabbb", "bbbccc"},
        {"hello", "world"}
    };
    for (int i = 0; i < 5; ++i) {
        char* out = longest(tests[i][0], tests[i][1]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("\n");
        }
    }
    return 0;
}