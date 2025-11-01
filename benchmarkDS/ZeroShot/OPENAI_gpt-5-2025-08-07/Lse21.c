#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

long long add_thousand(long long x) {
    return x + 1000;
}

static bool parse_long_long(const char* s, long long* out) {
    if (s == NULL || out == NULL) return false;
    errno = 0;
    char* end = NULL;
    long long val = strtoll(s, &end, 10);
    if (errno == ERANGE || end == s || (end != NULL && *end != '\0')) {
        return false;
    }
    *out = val;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        long long v = 0;
        if (parse_long_long(argv[1], &v)) {
            if (v <= LLONG_MAX - 1000 && v >= LLONG_MIN + 1000) {
                printf("%lld\n", add_thousand(v));
            } else {
                fprintf(stderr, "Overflow risk when adding 1000.\n");
            }
        } else {
            fprintf(stderr, "Invalid integer input.\n");
        }
    } else {
        long long tests[5] = {-1000, -1, 0, 1, 9223372036854774800LL};
        for (int i = 0; i < 5; ++i) {
            long long v = tests[i];
            if (v <= LLONG_MAX - 1000 && v >= LLONG_MIN + 1000) {
                printf("%lld\n", add_thousand(v));
            } else {
                printf("Overflow risk\n");
            }
        }
    }
    return 0;
}