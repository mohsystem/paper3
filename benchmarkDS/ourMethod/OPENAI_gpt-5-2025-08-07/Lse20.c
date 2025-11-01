#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>

#define OFFSET ((int64_t)1000)

int64_t add_offset_i64(int64_t value) {
    // Caller must ensure no overflow; this function assumes validated input.
    return value + OFFSET;
}

static int try_parse_i64(const char *s, int64_t *out) {
    if (s == NULL || out == NULL) return 0;
    // Trim leading/trailing whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\n' || s[len - 1] == '\r')) len--;
    if (len == 0 || len > 20) return 0; // optional sign + up to 19 digits

    char buf[32];
    if (len >= sizeof(buf)) return 0;
    memcpy(buf, s, len);
    buf[len] = '\0';

    // Basic format check
    size_t i = 0;
    if (buf[0] == '+' || buf[0] == '-') {
        if (len == 1) return 0;
        i = 1;
    }
    for (; i < len; ++i) {
        if (buf[i] < '0' || buf[i] > '9') return 0;
    }

    errno = 0;
    char *endptr = NULL;
    long long v = strtoll(buf, &endptr, 10);
    if (errno == ERANGE || endptr == buf || *endptr != '\0') {
        return 0;
    }
    // Ensure long long fits into int64_t (should for typical platforms)
    if (v < (long long)LLONG_MIN || v > (long long)LLONG_MAX) return 0;
    *out = (int64_t)v;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        int64_t input = 0;
        if (!try_parse_i64(argv[1], &input)) {
            fprintf(stderr, "Invalid input\n");
            return 1;
        }
        if (input > (INT64_MAX - OFFSET)) {
            fprintf(stderr, "Overflow\n");
            return 1;
        }
        int64_t result = add_offset_i64(input);
        printf("%" PRId64 "\n", result);
    } else {
        // 5 test cases
        int64_t tests[5] = {0, 42, -1000, LLONG_MAX - OFFSET, LLONG_MIN + OFFSET};
        for (size_t i = 0; i < 5; ++i) {
            if (tests[i] > (INT64_MAX - OFFSET)) {
                printf("Overflow\n");
            } else {
                int64_t r = add_offset_i64(tests[i]);
                printf("%" PRId64 "\n", r);
            }
        }
    }
    return 0;
}