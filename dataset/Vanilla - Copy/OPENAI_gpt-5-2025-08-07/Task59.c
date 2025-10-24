#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* accum(const char* s) {
    size_t n = strlen(s);
    size_t total = 0;
    for (size_t i = 0; i < n; ++i) {
        total += 1 + i; // first uppercase + i lowercase
        if (i != n - 1) total += 1; // hyphen
    }
    char* out = (char*)malloc(total + 1);
    if (!out) return NULL;

    size_t pos = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) out[pos++] = '-';
        unsigned char c = (unsigned char)s[i];
        out[pos++] = (char)toupper(c);
        for (size_t j = 0; j < i; ++j) {
            out[pos++] = (char)tolower(c);
        }
    }
    out[pos] = '\0';
    return out;
}

int main(void) {
    const char* tests[] = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "ZpglnRxqenU",
        "NyffsGeyylB"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char* res = accum(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}