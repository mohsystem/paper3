/* Chain-of-Through process:
 * 1) Problem understanding: Check if a letters-only string is an isogram, ignoring case; empty is true.
 * 2) Security requirements: Validate ASCII letters only, avoid buffer issues, no unsafe I/O.
 * 3) Secure coding generation: Manual lowercase conversion, track seen letters with fixed-size array.
 * 4) Code review: No external inputs, safe iteration, immediate fail on invalid char/duplicate.
 * 5) Secure code output: Final function returns 1 for true, 0 for false.
 */

#include <stdio.h>

int isIsogram(const char* s) {
    if (s == NULL) {
        return 1; /* Treat NULL as isogram for safety */
    }
    int seen[26] = {0};
    for (const unsigned char* p = (const unsigned char*)s; *p != '\0'; ++p) {
        unsigned char uc = *p;
        char c;
        if (uc >= 'A' && uc <= 'Z') {
            c = (char)(uc - 'A' + 'a');
        } else if (uc >= 'a' && uc <= 'z') {
            c = (char)uc;
        } else {
            /* Non-letter violates the letters-only assumption */
            return 0;
        }
        int idx = c - 'a';
        if (seen[idx]) return 0;
        seen[idx] = 1;
    }
    return 1;
}

static void runTest(const char* input) {
    int result = isIsogram(input);
    if (input == NULL) {
        printf("null --> %s\n", result ? "true" : "false");
    } else {
        printf("\"%s\" --> %s\n", input, result ? "true" : "false");
    }
}

int main(void) {
    runTest("Dermatoglyphics"); /* true */
    runTest("aba");             /* false */
    runTest("moOse");           /* false */
    runTest("");                /* true */
    runTest("isogram");         /* true */
    return 0;
}