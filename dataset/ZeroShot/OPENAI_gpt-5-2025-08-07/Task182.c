#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* dup_string(const char* s) {
    size_t len = s ? strlen(s) : 0;
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    if (len > 0) {
        memcpy(out, s, len);
    }
    out[len] = '\0';
    return out;
}

char* cleanNumber(const char* s) {
    if (s == NULL) {
        return dup_string("");
    }
    char digits[32];
    size_t n = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p != '\0'; ++p) {
        if (isdigit(*p)) {
            if (n < sizeof(digits) - 1) {
                digits[n++] = (char)(*p);
            } else {
                // Too many digits; mark invalid
                return dup_string("");
            }
        }
    }
    digits[n] = '\0';

    if (n == 11) {
        if (digits[0] != '1') {
            return dup_string("");
        }
        memmove(digits, digits + 1, 10);
        digits[10] = '\0';
        n = 10;
    }
    if (n != 10) {
        return dup_string("");
    }
    if (digits[0] < '2' || digits[0] > '9') {
        return dup_string("");
    }
    if (digits[3] < '2' || digits[3] > '9') {
        return dup_string("");
    }
    return dup_string(digits);
}

int main(void) {
    const char* tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "123-456-7890" /* invalid */
    };
    for (int i = 0; i < 5; ++i) {
        char* cleaned = cleanNumber(tests[i]);
        if (cleaned) {
            printf("%s\n", cleaned);
            free(cleaned);
        } else {
            // Allocation failure: print nothing but maintain line structure
            printf("\n");
        }
    }
    return 0;
}