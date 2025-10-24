/* Chain-of-Through process in code generation:
 * 1) Understand: Reverse words of length >= 5; preserve spaces exactly.
 * 2) Security: No unsafe I/O; careful memory allocation; no buffer overflow.
 * 3) Implementation: One pass scanning; allocate exact output size.
 * 4) Review: Validate allocations, free after use.
 * 5) Output: Include 5 test cases in main.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* spin_words(const char* s) {
    if (s == NULL) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;

    size_t i = 0, k = 0;
    while (i < n) {
        if (s[i] == ' ') {
            out[k++] = ' ';
            i++;
            continue;
        }
        size_t start = i;
        while (i < n && s[i] != ' ') i++;
        size_t len = i - start;
        if (len >= 5) {
            for (size_t j = 0; j < len; ++j) {
                out[k++] = s[i - 1 - j];
            }
        } else {
            memcpy(out + k, s + start, len);
            k += len;
        }
    }
    out[k] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "abcd efghi jklmn op qrstu vwxyz"
    };
    for (int idx = 0; idx < 5; ++idx) {
        char* res = spin_words(tests[idx]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("(null)\n");
        }
    }
    return 0;
}