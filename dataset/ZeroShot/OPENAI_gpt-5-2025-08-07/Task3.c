#include <stdio.h>

int is_pangram(const char *s) {
    if (s == NULL) return 0;
    unsigned int mask = 0u;
    const unsigned int FULL = (1u << 26) - 1u;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        unsigned char ch = *p++;
        if (ch >= 'A' && ch <= 'Z') ch = (unsigned char)(ch + 32);
        if (ch >= 'a' && ch <= 'z') {
            mask |= (1u << (ch - 'a'));
            if (mask == FULL) break;
        }
    }
    return mask == FULL ? 1 : 0;
}

int main(void) {
    const char *tests[5] = {
        "The quick brown fox jumps over the lazy dog",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs.",
        "The quick brown fox jumps over the lay dog"
    };
    for (int i = 0; i < 5; i++) {
        printf("%s\n", is_pangram(tests[i]) ? "True" : "False");
    }
    return 0;
}