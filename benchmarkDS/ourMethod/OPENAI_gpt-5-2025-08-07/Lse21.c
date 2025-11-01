#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

long long add1000(long long value) {
    return value + 1000LL;
}

static int is_valid_integer_string(const char *s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len == 0 || len > 21) return 0; // sign + up to 19 digits, conservative cap
    size_t i = 0;
    // no leading/trailing spaces allowed
    if (s[i] == '+' || s[i] == '-') i++;
    size_t digits = 0;
    for (; i < len; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
        digits++;
        if (digits > 19) return 0;
    }
    return digits >= 1;
}

static int parse_int64(const char *s, long long *out) {
    if (!is_valid_integer_string(s)) return 0;
    errno = 0;
    char *endptr = NULL;
    long long val = strtoll(s, &endptr, 10);
    if (errno == ERANGE || endptr == s || *endptr != '\0') return 0;
    *out = val;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        const char *input = argv[1] ? argv[1] : "";
        long long n = 0;
        if (!parse_int64(input, &n)) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        if (n > LLONG_MAX - 1000LL) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        long long result = add1000(n);
        printf("%lld\n", result);
        return 0;
    } else {
        // 5 test cases
        long long tests[5] = {
            0LL,
            1LL,
            -1000LL,
            123456789012345LL,
            LLONG_MAX - 1000LL
        };
        for (size_t i = 0; i < 5; i++) {
            long long t = tests[i];
            if (t > LLONG_MAX - 1000LL) {
                puts("overflow");
            } else {
                printf("%lld\n", add1000(t));
            }
        }
        return 0;
    }
}