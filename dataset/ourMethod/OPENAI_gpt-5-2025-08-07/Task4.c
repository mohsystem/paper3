#include <stdio.h>
#include <stdint.h>

static int is_ascii_letter(char c) {
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

static char to_lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return (char)(c - 'A' + 'a');
    }
    return c;
}

// Determines if the input string (letters only) is an isogram (case-insensitive).
// Returns 1 for true, 0 for false. Returns 0 if s is NULL or contains non-letters.
int is_isogram(const char* s) {
    if (s == NULL) {
        return 0; // invalid input
    }
    uint32_t seen = 0u;
    for (const char* p = s; *p != '\0'; ++p) {
        char ch = *p;
        if (!is_ascii_letter(ch)) {
            return 0; // input validation: only letters allowed
        }
        char lower = to_lower_ascii(ch);
        uint32_t bit = 1u << (unsigned)(lower - 'a');
        if ((seen & bit) != 0u) {
            return 0; // repeated letter
        }
        seen |= bit;
    }
    return 1; // isogram
}

int main(void) {
    const char* tests[5] = {
        "Dermatoglyphics", /* true */
        "aba",             /* false */
        "moOse",           /* false */
        "",                /* true */
        "isogram"          /* true */
    };

    for (int i = 0; i < 5; ++i) {
        int res = is_isogram(tests[i]);
        printf("Input: \"%s\" -> %s\n", tests[i], res ? "true" : "false");
    }

    return 0;
}