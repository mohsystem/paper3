#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

typedef struct {
    int success;
    long long value;
} AddResult;

static int is_valid_integer_string(const char *s, size_t max_len) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len == 0 || len > max_len) return 0;
    size_t start = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (len == 1) return 0;
        start = 1;
    }
    for (size_t i = start; i < len; ++i) {
        unsigned char ch = (unsigned char)s[i];
        if (!isdigit(ch)) return 0;
    }
    return 1;
}

static int parse_long_long(const char *s, long long *out) {
    if (!is_valid_integer_string(s, 100)) return 0;
    errno = 0;
    char *endptr = NULL;
    long long val = strtoll(s, &endptr, 10);
    if (errno != 0 || endptr == s || *endptr != '\0') {
        return 0;
    }
    *out = val;
    return 1;
}

AddResult add_thousand(long long input) {
    AddResult result;
    if (input > LLONG_MAX - 1000LL || input < LLONG_MIN + 1000LL) {
        result.success = 0;
        result.value = 0;
        return result;
    }
    result.success = 1;
    result.value = input + 1000LL;
    return result;
}

int main(int argc, char *argv[]) {
    // 5 test cases
    const char *tests[] = {
        "-1000",
        "0",
        "42",
        "2147483647",
        "9223372036854774807"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        long long val = 0;
        if (!parse_long_long(tests[i], &val)) {
            printf("Invalid test input: %s\n", tests[i]);
        } else {
            AddResult r = add_thousand(val);
            if (r.success) {
                printf("add_thousand(%s) = %lld\n", tests[i], r.value);
            } else {
                printf("Overflow in test input: %s\n", tests[i]);
            }
        }
    }

    // Command-line processing
    if (argc >= 2) {
        long long val = 0;
        if (!parse_long_long(argv[1], &val)) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        AddResult r = add_thousand(val);
        if (!r.success) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        printf("%lld\n", r.value);
    }
    return 0;
}