#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>

// Chain-of-Through Step 1: Problem understanding
// Convert an array of 0/1 bits (MSB first) to an integer.

// Chain-of-Through Step 2: Security requirements
// - Validate non-binary values.
// - Detect and prevent integer overflow.

// Chain-of-Through Step 3: Secure coding generation
int binaryArrayToInt(const int* bits, size_t len) {
    if (bits == NULL) {
        return -1; // indicate error
    }
    int acc = 0;
    for (size_t i = 0; i < len; ++i) {
        int b = bits[i];
        if (b != 0 && b != 1) {
            return -1; // non-binary value
        }
        // Check overflow: acc = acc*2 + b
        if (acc > (INT_MAX - b) / 2) {
            return -1; // overflow
        }
        acc = (acc << 1) | b;
    }
    return acc;
}

// Chain-of-Through Step 4 and 5: Review and finalize (no known vulnerabilities)
int main(void) {
    int t1[] = {0, 0, 0, 1}; // 1
    int t2[] = {0, 0, 1, 0}; // 2
    int t3[] = {0, 1, 0, 1}; // 5
    int t4[] = {1, 0, 0, 1}; // 9
    int t5[] = {1, 1, 1, 1}; // 15

    int* tests[] = {t1, t2, t3, t4, t5};
    size_t lens[] = {4, 4, 4, 4, 4};

    for (size_t i = 0; i < 5; ++i) {
        int result = binaryArrayToInt(tests[i], lens[i]);
        printf("Testing: [");
        for (size_t j = 0; j < lens[i]; ++j) {
            printf("%d%s", tests[i][j], (j + 1 < lens[i]) ? ", " : "");
        }
        printf("] ==> %d\n", result);
    }

    return 0;
}