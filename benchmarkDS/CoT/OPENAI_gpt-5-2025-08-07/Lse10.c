/*
Chain-of-Through process:
1) Problem understanding: Provide getValueFromArray and a main that reads an int, calls it with id_sequence, exits with returned value.
2) Security requirements: Validate inputs, size, and bounds; sanitize exit codes; handle malformed input.
3) Secure coding: Check pointers and bounds; set errno on invalid input; avoid UB.
4) Code review: Ensured checks and predictable behavior.
5) Secure output: Provide test mode when no input is given.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

int getValueFromArray(const int* arr, size_t size, int index) {
    if (arr == NULL) { errno = EINVAL; return 0; }
    if (index < 0 || (size_t)index >= size) { errno = ERANGE; return 0; }
    errno = 0;
    return arr[(size_t)index];
}

static int to_exit_code(int value) {
    return ((unsigned int)value) & 0xFF; // 0..255
}

static void run_tests(void) {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70};
    size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    int tests[5] = {0, 3, 4, 6, 10}; // last is out-of-bounds
    printf("Test mode: demonstrating 5 test cases\n");
    for (int i = 0; i < 5; ++i) {
        errno = 0;
        int idx = tests[i];
        int val = getValueFromArray(id_sequence, size, idx);
        if (errno != 0) {
            printf("Index %d -> error: %d\n", idx, errno);
        } else {
            printf("Index %d -> %d\n", idx, val);
        }
    }
}

int main(void) {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70};
    size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    int idx;
    if (scanf("%d", &idx) == 1) {
        int val = getValueFromArray(id_sequence, size, idx);
        if (errno != 0) {
            fprintf(stderr, "Error: invalid index or input\n");
            return 1;
        }
        return to_exit_code(val);
    } else {
        // No integer provided; run 5 test cases
        run_tests();
        return 0;
    }
}