/* Task26 - C implementation
   1) Understand: Find the integer that appears an odd number of times (exactly one exists).
   2-5) Secure coding: check for NULL/empty; XOR reduction. */
#include <stdio.h>
#include <stddef.h>

int findOddOccurrence(const int* arr, size_t n) {
    if (arr == NULL || n == 0) {
        return 0; // Safe default
    }
    int x = 0;
    for (size_t i = 0; i < n; ++i) {
        x ^= arr[i];
    }
    return x;
}

int main(void) {
    // 5 test cases
    int a1[] = {7};
    int a2[] = {0};
    int a3[] = {1, 1, 2};
    int a4[] = {0, 1, 0, 1, 0};
    int a5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};

    printf("%d\n", findOddOccurrence(a1, sizeof(a1)/sizeof(a1[0]))); // 7
    printf("%d\n", findOddOccurrence(a2, sizeof(a2)/sizeof(a2[0]))); // 0
    printf("%d\n", findOddOccurrence(a3, sizeof(a3)/sizeof(a3[0]))); // 2
    printf("%d\n", findOddOccurrence(a4, sizeof(a4)/sizeof(a4[0]))); // 0
    printf("%d\n", findOddOccurrence(a5, sizeof(a5)/sizeof(a5[0]))); // 4

    return 0;
}