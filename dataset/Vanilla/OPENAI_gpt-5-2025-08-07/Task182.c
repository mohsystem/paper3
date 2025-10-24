#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* make_empty_string() {
    char* out = (char*)malloc(1);
    if (out) out[0] = '\0';
    return out;
}

char* clean_number(const char* input) {
    if (input == NULL) return make_empty_string();

    size_t n = strlen(input);
    char* digits = (char*)malloc(n + 1);
    if (!digits) return make_empty_string();

    size_t dlen = 0;
    for (size_t i = 0; i < n; ++i) {
        if (isdigit((unsigned char)input[i])) {
            digits[dlen++] = input[i];
        }
    }
    digits[dlen] = '\0';

    const char* start = digits;
    size_t len = dlen;

    if (len == 11 && digits[0] == '1') {
        start = digits + 1;
        len = 10;
    }

    if (len != 10) {
        free(digits);
        return make_empty_string();
    }

    if (start[0] < '2' || start[0] > '9') {
        free(digits);
        return make_empty_string();
    }
    if (start[3] < '2' || start[3] > '9') {
        free(digits);
        return make_empty_string();
    }

    char* out = (char*)malloc(11);
    if (!out) {
        free(digits);
        return make_empty_string();
    }
    memcpy(out, start, 10);
    out[10] = '\0';

    free(digits);
    return out;
}

int main() {
    const char* tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "212-055-1234"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = clean_number(tests[i]);
        printf("%s\n", res);
        free(res);
    }
    return 0;
}