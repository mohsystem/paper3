#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define OFFSET 1000LL

long long add_offset(long long value, int* error) {
    if (value > LLONG_MAX - OFFSET) {
        if (error) *error = 1;
        return 0;
    }
    if (value < LLONG_MIN + OFFSET) {
        if (error) *error = 1;
        return 0;
    }
    if (error) *error = 0;
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        errno = 0;
        char* end = NULL;
        long long v = strtoll(argv[1], &end, 10);
        if (errno == ERANGE || end == argv[1] || (end && *end != '\0')) {
            fprintf(stderr, "Error: invalid integer input\n");
            return 1;
        }
        int err = 0;
        long long res = add_offset(v, &err);
        if (err) {
            fprintf(stderr, "Error: overflow/underflow when adding offset\n");
            return 1;
        }
        printf("%lld\n", res);
    } else {
        long long tests[5] = {0LL, 1LL, -1LL, LLONG_MAX - 1000LL, LLONG_MIN + 1000LL};
        for (int i = 0; i < 5; ++i) {
            int err = 0;
            long long r = add_offset(tests[i], &err);
            if (err) {
                printf("add_offset(%lld) error: overflow/underflow\n", tests[i]);
            } else {
                printf("add_offset(%lld) = %lld\n", tests[i], r);
            }
        }
    }
    return 0;
}