/* Chain-of-Through process via comments for secure coding.
   1) Problem: reverse a C-string.
   2) Security: handle NULL, check allocation, avoid overflow.
   3) Secure generation: allocate len+1, check failures.
   4) Review: no off-by-one, proper free in main.
   5) Final secure output below.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

char* reverse_string(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    size_t len = strlen(input);
    if (len > SIZE_MAX - 1) {
        return NULL; /* overflow guard */
    }
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        out[i] = input[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {"", "hello", "A", "racecar", "Hello, World!"};
    for (int i = 0; i < 5; ++i) {
        char* r = reverse_string(tests[i]);
        if (r) {
            printf("Input: \"%s\" -> Reversed: \"%s\"\n", tests[i], r);
            free(r);
        } else {
            printf("Input: \"%s\" -> Reversed: %s\n", tests[i], "(null)");
        }
    }
    return 0;
}