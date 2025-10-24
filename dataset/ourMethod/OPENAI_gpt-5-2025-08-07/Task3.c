#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

// Detects if the input string is a pangram (contains every letter A-Z at least once, case-insensitive).
// Parameters: s - input C-string (null-terminated). If NULL, returns 0 (False).
// Returns: 1 if pangram, 0 otherwise.
int is_pangram(const char *s) {
    if (s == NULL) {
        return 0; // Fail closed on invalid input
    }

    const uint32_t allMask = (1u << 26) - 1u; // 26 ones (0x03FFFFFF)
    uint32_t mask = 0u;

    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char uc = (unsigned char)s[i];
        if (isalpha(uc)) {
            int lower = tolower(uc);
            int idx = lower - 'a';
            if (idx >= 0 && idx < 26) {
                mask |= (1u << idx);
                if (mask == allMask) {
                    return 1; // Early exit if all letters found
                }
            }
        }
    }
    return mask == allMask ? 1 : 0;
}

int main(void) {
    const char *tests[5] = {
        "The quick brown fox jumps over the lazy dog",
        "Sphinx of black quartz, judge my vow!",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs 12345!!!"
    };

    for (int i = 0; i < 5; ++i) {
        int result = is_pangram(tests[i]);
        printf("%s\n", result ? "True" : "False");
    }

    return 0;
}