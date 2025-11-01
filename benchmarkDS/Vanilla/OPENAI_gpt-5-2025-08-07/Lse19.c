#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

long long add1000(long long x) {
    return x + 1000;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* endptr = NULL;
        errno = 0;
        long long val = strtoll(argv[1], &endptr, 10);
        if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Invalid number: %s\n", argv[1]);
        } else {
            printf("%lld\n", add1000(val));
        }
    }
    long long tests[5] = {
        -1000LL,
        0LL,
        42LL,
        999999999999LL,
        9223372036854774807LL
    };
    for (int i = 0; i < 5; ++i) {
        printf("%lld\n", add1000(tests[i]));
    }
    return 0;
}