#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int add1000(int x) {
    return x + 1000;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        char *endptr = NULL;
        errno = 0;
        long val = strtol(argv[1], &endptr, 10);
        if (errno == 0 && endptr != argv[1] && *endptr == '\0') {
            int result = add1000((int)val);
            printf("%d\n", result);
            return 0;
        }
        // Fall through to test cases on invalid input
    }
    int tests[5] = { -1000, -1, 0, 1, 12345 };
    for (int i = 0; i < 5; ++i) {
        printf("%d\n", add1000(tests[i]));
    }
    return 0;
}