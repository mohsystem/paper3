#include <stdio.h>
#include <string.h>

#define MAX_LEN 64

static int is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v');
}

// Secure integer parsing: returns 1 on success with *out set, 0 on failure
int parse_to_int(const char* input, int* out) {
    if (input == NULL || out == NULL) return 0;

    // Trim leading spaces
    const char* start = input;
    while (*start && is_space(*start)) start++;
    if (*start == '\0') return 0;

    // Trim trailing spaces by finding end
    const char* end = start + strlen(start);
    while (end > start && is_space(*(end - 1))) end--;

    size_t len = (size_t)(end - start);
    if (len == 0 || len > MAX_LEN) return 0;

    size_t i = 0;
    int negative = 0;

    if (start[0] == '-') { negative = 1; i = 1; }
    else if (start[0] == '+') { i = 1; }

    if (i >= len) return 0;

    int limit = negative ? (-2147483647 - 1) : -2147483647;
    int multmin = limit / 10;
    int result = 0;

    for (; i < len; ++i) {
        char ch = start[i];
        if (ch < '0' || ch > '9') return 0;
        int digit = ch - '0';
        if (result < multmin) return 0;
        result *= 10;
        if (result < limit + digit) return 0;
        result -= digit;
    }

    *out = negative ? result : -result;
    return 1;
}

int main(int argc, char* argv[]) {
    const char* defaults[5] = {"123", "-42", "00123", "2147483648", "abc123"};
    int use_defaults = (argc <= 1);

    if (use_defaults) {
        for (int i = 0; i < 5; ++i) {
            int value = 0;
            if (parse_to_int(defaults[i], &value)) {
                printf("Input: \"%s\" -> OK: %d\n", defaults[i], value);
            } else {
                printf("Input: \"%s\" -> Invalid integer\n", defaults[i]);
            }
        }
    } else {
        for (int i = 1; i < argc; ++i) {
            int value = 0;
            if (parse_to_int(argv[i], &value)) {
                printf("Input: \"%s\" -> OK: %d\n", argv[i], value);
            } else {
                printf("Input: \"%s\" -> Invalid integer\n", argv[i]);
            }
        }
    }
    return 0;
}