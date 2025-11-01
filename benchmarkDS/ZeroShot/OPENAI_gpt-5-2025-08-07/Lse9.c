#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const char* arg_at_index(long long index, int argc, char* argv[]) {
    if (index < 0 || index >= argc) return NULL;
    return argv[index];
}

static int parse_non_negative_ll(const char* s, long long* out) {
    if (!s || !out) return 0;
    errno = 0;
    char* endptr = NULL;
    long long val = strtoll(s, &endptr, 10);
    if (errno != 0 || endptr == s || *endptr != '\0') return 0;
    if (val < 0) return 0;
    *out = val;
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        long long idx = -1;
        if (!parse_non_negative_ll(argv[1], &idx)) {
            fprintf(stderr, "Error: index must be a non-negative integer.\n");
            return 1;
        }
        if (idx < 0 || idx >= argc) {
            fprintf(stderr, "Error: index out of range.\n");
            return 1;
        }
        const char* res = arg_at_index(idx, argc, argv);
        if (res) {
            printf("%s\n", res);
        }
        return 0;
    }

    // Test cases (run when no CLI args are provided)
    {
        char* v1[] = {"prog", "1", "alpha", "beta", "gamma"};
        const char* r1 = arg_at_index(0, 5, v1);
        printf("%s\n", r1 ? r1 : "");

        const char* r2 = arg_at_index(1, 5, v1);
        printf("%s\n", r2 ? r2 : "");

        const char* r3 = arg_at_index(2, 5, v1);
        printf("%s\n", r3 ? r3 : "");

        char* v2[] = {"prog", "3", "foo", "bar", "baz"};
        const char* r4 = arg_at_index(3, 5, v2);
        printf("%s\n", r4 ? r4 : "");

        char* v3[] = {"prog", "0"};
        const char* r5 = arg_at_index(0, 2, v3);
        printf("%s\n", r5 ? r5 : "");
    }

    return 0;
}