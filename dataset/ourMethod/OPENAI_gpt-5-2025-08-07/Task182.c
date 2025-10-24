#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* clean_number(const char* s) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // Limit maximum input length to mitigate excessive memory usage
    const size_t MAX_INPUT = 4096;
    size_t in_len = 0;
    while (s[in_len] != '\0' && in_len < MAX_INPUT) {
        in_len++;
    }
    if (in_len >= MAX_INPUT) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    char* digits = (char*)malloc(in_len + 1);
    if (!digits) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t dlen = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char uc = (unsigned char)s[i];
        if (isdigit(uc)) {
            digits[dlen++] = (char)uc;
        }
    }
    digits[dlen] = '\0';

    // Handle country code
    if (dlen == 11 && digits[0] == '1') {
        // shift left by one
        memmove(digits, digits + 1, dlen - 1);
        dlen -= 1;
        digits[dlen] = '\0';
    } else if (dlen != 10) {
        free(digits);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // Validate NANP NXX NXX-XXXX where N is 2-9
    if (digits[0] < '2' || digits[0] > '9' || digits[3] < '2' || digits[3] > '9') {
        free(digits);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    char* out = (char*)malloc(11);
    if (!out) {
        free(digits);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    memcpy(out, digits, 10);
    out[10] = '\0';
    free(digits);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "1 (123) 456-7890"
    };
    for (int i = 0; i < 5; i++) {
        char* cleaned = clean_number(tests[i]);
        if (cleaned) {
            printf("%s\n", cleaned);
            free(cleaned);
        } else {
            printf("\n");
        }
    }
    return 0;
}