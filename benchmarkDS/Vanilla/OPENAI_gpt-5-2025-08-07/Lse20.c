#include <stdio.h>
#include <stdlib.h>

static const int OFFSET = 1000;

int add_offset(int value) {
    return value + OFFSET;
}

void run_tests() {
    int tests[5] = {0, 1, -1, 42, 999};
    for (int i = 0; i < 5; ++i) {
        printf("Test %d -> %d\n", tests[i], add_offset(tests[i]));
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* endptr = NULL;
        long v = strtol(argv[1], &endptr, 10);
        if (endptr != argv[1] && *endptr == '\0') {
            printf("%d\n", add_offset((int)v));
        } else {
            printf("Invalid integer input\n");
        }
    }
    run_tests();
    return 0;
}