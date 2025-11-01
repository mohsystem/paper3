#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

const char* arg_at_index(int argc, const char* const argv[], long index) {
    if (argv == NULL || argc < 0) {
        return NULL;
    }
    if (index < 0 || index >= argc) {
        return NULL;
    }
    return argv[index];
}

static void run_tests(void) {
    const char* sample[] = { "prog", "alpha", "beta", "gamma", "delta" };
    int sample_argc = (int)(sizeof(sample) / sizeof(sample[0]));

    long tests[5] = { 0, 1, 3, 4, 10 }; /* 5 test cases */
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        const char* res = arg_at_index(sample, sample_argc, tests[i]);
        if (res != NULL) {
            printf("%s\n", res);
        } else {
            printf("ERROR\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        errno = 0;
        char* endptr = NULL;
        long idx = strtol(argv[1], &endptr, 10);
        if (errno == ERANGE || endptr == argv[1] || (endptr != NULL && *endptr != '\0') || idx < 0) {
            printf("ERROR\n");
            return 0;
        }
        const char* res = arg_at_index(argc, (const char* const*)argv, idx);
        if (res != NULL) {
            printf("%s\n", res);
        } else {
            printf("ERROR\n");
        }
    } else {
        /* No args provided: run 5 test cases */
        run_tests();
    }
    return 0;
}