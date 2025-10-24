#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

// Converts an array of binary digits to its integer equivalent.
//
// @param arr Pointer to the input array of integers (0s and 1s).
// @param size The number of elements in the array.
// @return The integer value of the binary representation.
//         On error (NULL pointer, invalid digit, or overflow), it prints a
//         message to stderr and returns ULLONG_MAX. Note that ULLONG_MAX is
//         a valid output for an input of 64 '1's, making the error case
//         ambiguous.
unsigned long long binaryArrayToInteger(const int* arr, size_t size) {
    // Input validation: Check for NULL pointer if size is not zero.
    if (arr == NULL && size > 0) {
        fprintf(stderr, "Error: Input array pointer is NULL for non-zero size.\n");
        return ULLONG_MAX;
    }

    unsigned long long result = 0;
    for (size_t i = 0; i < size; ++i) {
        int bit = arr[i];
        // Input validation: ensure each element is a valid binary digit.
        if (bit != 0 && bit != 1) {
            fprintf(stderr, "Error: Input array contains non-binary value.\n");
            return ULLONG_MAX;
        }
        // Security check: prevent integer overflow before multiplication.
        if (result > (ULLONG_MAX - bit) / 2) {
            fprintf(stderr, "Error: Overflow would occur during conversion.\n");
            return ULLONG_MAX;
        }
        result = (result << 1) | bit;
    }
    return result;
}

// Helper function to run and print a test case.
void run_test(const int* arr, size_t size) {
    printf("Testing: [");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("] ==> %llu\n", binaryArrayToInteger(arr, size));
}

int main() {
    // 5 Test cases
    int test1[] = {0, 0, 0, 1};
    run_test(test1, sizeof(test1) / sizeof(test1[0]));

    int test2[] = {0, 0, 1, 0};
    run_test(test2, sizeof(test2) / sizeof(test2[0]));

    int test3[] = {0, 1, 0, 1};
    run_test(test3, sizeof(test3) / sizeof(test3[0]));

    int test4[] = {1, 0, 0, 1};
    run_test(test4, sizeof(test4) / sizeof(test4[0]));

    int test5[] = {0, 1, 1, 0};
    run_test(test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}