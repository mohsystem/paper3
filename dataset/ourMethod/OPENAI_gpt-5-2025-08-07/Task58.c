#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// Check if the string has the same amount of 'x' and 'o' (case-insensitive).
// Returns true if counts are equal (including when both are zero), false otherwise.
bool XO(const char *s) {
    if (s == NULL) {
        return false; // fail closed on invalid input
    }

    // Basic input validation: bound the length to avoid excessive processing
    const size_t MAX_LEN = 10000000u;
    size_t len = strnlen(s, MAX_LEN + 1u);
    if (len > MAX_LEN) {
        return false; // input too large
    }

    size_t countX = 0u;
    size_t countO = 0u;
    for (size_t i = 0u; i < len; ++i) {
        unsigned char ch = (unsigned char)s[i];
        unsigned char lower = (unsigned char)tolower(ch);
        if (lower == (unsigned char)'x') {
            ++countX;
        } else if (lower == (unsigned char)'o') {
            ++countO;
        }
    }
    return countX == countO;
}

int main(void) {
    const char *tests[5] = {
        "ooxx",
        "xooxx",
        "ooxXm",
        "zpzpzpp",
        "zzoo"
    };

    for (int i = 0; i < 5; ++i) {
        bool result = XO(tests[i]);
        printf("XO(\"%s\") => %s\n", tests[i], result ? "true" : "false");
    }

    return 0;
}