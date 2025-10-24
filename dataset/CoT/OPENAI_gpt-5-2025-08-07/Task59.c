/* Chain-of-Through Step 1: Problem understanding
   Implement accum(s) => "A-Bb-Ccc-...".
   Step 2: Security requirements
   - Validate input pointer.
   - Safe memory allocation with overflow checks.
   - Use unsigned char for ctype functions.
   Step 3: Secure coding generation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

char* accum(const char* s) {
    if (s == NULL) return NULL;
    size_t n = strlen(s);
    if (n == 0) {
        char* out = (char*)malloc(1u);
        if (out) out[0] = '\0';
        return out;
    }
    /* Compute required size: letters = n + n*(n-1)/2; hyphens = n-1; +1 for '\0' */
    unsigned long long letters = (unsigned long long)n + ((unsigned long long)n * ((unsigned long long)n - 1ull)) / 2ull;
    unsigned long long hyphens = (unsigned long long)(n - 1u);
    unsigned long long total = letters + hyphens + 1ull;
    if (total > (unsigned long long)SIZE_MAX) {
        return NULL; /* overflow risk */
    }

    char* out = (char*)malloc((size_t)total);
    if (!out) return NULL;

    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) out[idx++] = '-';
        unsigned char uc = (unsigned char)s[i];
        out[idx++] = (char)toupper(uc);
        char lower = (char)tolower(uc);
        for (size_t j = 0; j < i; ++j) {
            out[idx++] = lower;
        }
    }
    out[idx] = '\0';
    return out;
}

/* Step 4: Code review
   - idx never exceeds allocated total-1 due to formula.
   - Checked malloc and overflow.
   Step 5: Secure code output - tests
*/
int main(void) {
    const char* tests[5] = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "",
        "ZpglnRxqenU"
    };
    for (int i = 0; i < 5; ++i) {
        char* r = accum(tests[i]);
        if (r) {
            printf("accum(\"%s\") -> \"%s\"\n", tests[i], r);
            free(r);
        } else {
            printf("accum(\"%s\") -> <allocation failed>\n", tests[i]);
        }
    }
    return 0;
}