#include <stdio.h>
#include <stdint.h>

int is_isogram(const char* s) {
    if (s == NULL) return 0;

    // Compute length up to 27 to check quickly for >26
    int len = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        len++;
        if (len > 26) return 0; // Pigeonhole principle for English alphabet
    }
    if (len == 0) return 1;

    uint32_t mask = 0U;
    for (int i = 0; s[i] != '\0'; ++i) {
        unsigned char uc = (unsigned char)s[i];
        char c = (char)uc;
        if (c >= 'A' && c <= 'Z') {
            c = (char)(c + ('a' - 'A'));
        } else if (!(c >= 'a' && c <= 'z')) {
            return 0; // invalid character (non-letter)
        }
        int idx = c - 'a';
        uint32_t bit = 1U << idx;
        if (mask & bit) return 0;
        mask |= bit;
    }
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };

    for (int i = 0; i < 5; ++i) {
        int res = is_isogram(tests[i]);
        printf("%s -> %s\n", tests[i], res ? "true" : "false");
    }
    return 0;
}