/* Task182 - C implementation
   Chain-of-Through process:
   1) Problem: Clean NANP numbers by stripping punctuation and optional '1' country code; enforce NXX NXX-XXXX.
   2) Security: Validate inputs, avoid buffer overflows, check allocations.
   3) Secure coding: Two-pass approach, malloc with checks, careful index use.
   4) Review: Validate length and leading digit constraints.
   5) Output: Return allocated cleaned string or empty string on invalid. Caller must free.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* dup_empty_string(void) {
    char* s = (char*)malloc(1);
    if (s) s[0] = '\0';
    return s;
}

char* clean(const char* input) {
    if (input == NULL) {
        return dup_empty_string();
    }
    size_t in_len = strlen(input);
    // Temporary buffer to collect digits
    char* tmp = (char*)malloc(in_len + 1);
    if (!tmp) return NULL; // allocation failure
    size_t dcount = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (isdigit(c)) {
            tmp[dcount++] = (char)c;
        }
    }
    tmp[dcount] = '\0';

    // Handle country code
    if (dcount == 11) {
        if (tmp[0] != '1') {
            free(tmp);
            return dup_empty_string();
        }
        // Shift left by one to remove leading '1'
        memmove(tmp, tmp + 1, dcount - 1);
        dcount = 10;
        tmp[dcount] = '\0';
    }
    if (dcount != 10) {
        free(tmp);
        return dup_empty_string();
    }
    if (tmp[0] < '2' || tmp[0] > '9') {
        free(tmp);
        return dup_empty_string();
    }
    if (tmp[3] < '2' || tmp[3] > '9') {
        free(tmp);
        return dup_empty_string();
    }

    // Allocate final result
    char* out = (char*)malloc(11);
    if (!out) {
        free(tmp);
        return NULL;
    }
    memcpy(out, tmp, 10);
    out[10] = '\0';
    free(tmp);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "1 123 456 7890"
    };
    for (int i = 0; i < 5; i++) {
        char* res = clean(tests[i]);
        if (res == NULL) {
            // Allocation failed; print nothing or an indicator
            // For safety, print empty line
            printf("\n");
        } else {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}