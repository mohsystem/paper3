#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

// Safely adds 1000 to a long long. Returns 1 on success, 0 on overflow.
int safe_add1000(long long in, long long* out) {
    if (out == NULL) return 0;
    if (in > LLONG_MAX - 1000) {
        return 0; // would overflow
    }
    *out = in + 1000;
    return 1;
}

// Parse signed long long from string with validation (base 10). Returns 1 on success, 0 on failure.
int parse_ll(const char* input, long long* out) {
    if (input == NULL || out == NULL) return 0;

    // Trim leading whitespace
    while (isspace((unsigned char)*input)) input++;

    if (*input == '\0') return 0;

    errno = 0;
    char* endptr = NULL;
    long long val = strtoll(input, &endptr, 10);
    if (errno == ERANGE) return 0;

    // Trim trailing whitespace
    while (endptr && *endptr != '\0' && isspace((unsigned char)*endptr)) endptr++;

    if (endptr == NULL || *endptr != '\0') return 0;

    *out = val;
    return 1;
}

int main(int argc, char* argv[]) {
    // Process command line input: read a value, add 1000, print result
    if (argc > 1 && argv[1] != NULL) {
        long long v = 0, r = 0;
        if (parse_ll(argv[1], &v)) {
            if (safe_add1000(v, &r)) {
                printf("%lld\n", r);
            } else {
                fprintf(stderr, "Overflow when adding 1000\n");
            }
        } else {
            fprintf(stderr, "Invalid integer input\n");
        }
    }

    // 5 test cases
    long long tests[5];
    tests[0] = 0;
    tests[1] = 12345;
    tests[2] = -1000;
    tests[3] = LLONG_MAX - 999; // safe
    tests[4] = LLONG_MAX - 998; // will overflow

    for (int i = 0; i < 5; ++i) {
        long long out = 0;
        if (safe_add1000(tests[i], &out)) {
            printf("%lld\n", out);
        } else {
            fprintf(stderr, "Test overflow for input: %lld\n", tests[i]);
        }
    }

    return 0;
}