#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

const char* get_arg_at_index(int argc, char* argv[], int index) {
    if (index < 0 || index >= argc) {
        return NULL;
    }
    return argv[index];
}

static int parse_index(const char* s, int* out) {
    if (s == NULL || out == NULL) return 0;
    errno = 0;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    if (v < 0 || v > INT_MAX) return 0;
    *out = (int)v;
    return 1;
}

static void run_tests(void) {
    char* testArgv[] = {"prog", "1", "alpha", "beta", "gamma"};
    int testArgc = (int)(sizeof(testArgv) / sizeof(testArgv[0]));
    int tests[5] = {1, 2, 4, 0, 10};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        int idx = tests[i];
        const char* res = get_arg_at_index(testArgc, testArgv, idx);
        if (res != NULL) {
            printf("%s\n", res);
        } else {
            printf("ERROR\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        run_tests();
        return 0;
    }
    int idx = -1;
    if (!parse_index(argv[1], &idx)) {
        fprintf(stderr, "Error: invalid index\n");
        return 1;
    }
    if (idx < 0) {
        fprintf(stderr, "Error: index must be non-negative\n");
        return 1;
    }
    const char* result = get_arg_at_index(argc, argv, idx);
    if (result == NULL) {
        fprintf(stderr, "Error: index out of range\n");
        return 1;
    }
    printf("%s\n", result);
    return 0;
}