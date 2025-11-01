#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define OFFSET 1000LL

long long add_offset(long long value) {
    if (value > LLONG_MAX - OFFSET) {
        fprintf(stderr, "Error: Overflow when adding offset\n");
        exit(EXIT_FAILURE);
    }
    return value + OFFSET;
}

static void run_tests(void) {
    long long tests[5] = {0LL, 1LL, 999LL, -1000LL, 1234567890LL};
    for (int i = 0; i < 5; ++i) {
        long long t = tests[i];
        long long result = add_offset(t);
        printf("Test input: %lld -> %lld\n", t, result);
    }
}

int main(int argc, char* argv[]) {
    run_tests();

    if (argc > 1) {
        char* endptr = NULL;
        errno = 0;
        long long value = strtoll(argv[1], &endptr, 10);
        if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Invalid number format: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
        long long result = add_offset(value);
        printf("Command-line input: %lld -> %lld\n", value, result);
    }

    return 0;
}