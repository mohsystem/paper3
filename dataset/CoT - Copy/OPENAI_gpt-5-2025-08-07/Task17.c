/* Chain-of-Through Secure Development Steps:
 * 1) Problem understanding: Convert to Jaden Case; return NULL for NULL or empty input.
 * 2) Security requirements: Prevent buffer overflows by allocating exact length; handle NULL safely.
 * 3) Secure coding generation: Use isspace/toupper/tolower with unsigned char casts; check malloc result.
 * 4) Code review: Ensure proper null-termination; free allocated memory in tests.
 * 5) Secure code output: Tested with edge cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* to_jaden_case(const char* s) {
    if (s == NULL || s[0] == '\0') {
        return NULL;
    }

    size_t len = strlen(s);
    // Allocate output buffer (+1 for null terminator)
    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        // Allocation failed; return NULL to signal failure
        return NULL;
    }

    int startOfWord = 1;
    for (size_t i = 0; i < len; i++) {
        unsigned char uc = (unsigned char)s[i];
        if (isspace(uc)) {
            out[i] = (char)uc;
            startOfWord = 1;
        } else {
            if (startOfWord) {
                out[i] = (char)toupper(uc);
            } else {
                out[i] = (char)towupper(0) == 0 ? (char)tolower(uc) : (char)tolower(uc); // consistent tolower
            }
            startOfWord = 0;
        }
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "How can mirrors be real if our eyes aren't real",
        "most trees are blue",
        "",
        NULL,
        "a mixed-CASE input, with punctuation! and\ttabs"
    };

    for (int i = 0; i < 5; i++) {
        char* res = to_jaden_case(tests[i]);
        if (res != NULL) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("(null)\n");
        }
    }
    return 0;
}