#include <stdio.h>
#include <stddef.h>

// Secure implementation: returns -1 on invalid input
int findMissingNumber(const int* arr, size_t size, int n) {
    if (n < 1) return -1;
    if (size != (size_t)(n - 1)) return -1;

    int xorFull = 0;
    for (int i = 1; i <= n; ++i) {
        xorFull ^= i;
    }

    int xorArr = 0;
    if (size > 0) {
        if (arr == NULL) return -1;
        for (size_t i = 0; i < size; ++i) {
            xorArr ^= arr[i];
        }
    }
    return xorFull ^ xorArr;
}

int main(void) {
    // Test case 1: n=5, missing 4
    int t1[] = {1, 2, 3, 5};
    printf("%d\n", findMissingNumber(t1, sizeof(t1)/sizeof(t1[0]), 5)); // Expected 4

    // Test case 2: n=1, missing 1 (empty array)
    printf("%d\n", findMissingNumber(NULL, 0, 1)); // Expected 1

    // Test case 3: n=10, missing 7
    int t3[] = {1, 2, 3, 4, 5, 6, 8, 9, 10};
    printf("%d\n", findMissingNumber(t3, sizeof(t3)/sizeof(t3[0]), 10)); // Expected 7

    // Test case 4: n=2, missing 1
    int t4[] = {2};
    printf("%d\n", findMissingNumber(t4, sizeof(t4)/sizeof(t4[0]), 2)); // Expected 1

    // Test case 5: n=8, missing 8
    int t5[] = {1, 2, 3, 4, 5, 6, 7};
    printf("%d\n", findMissingNumber(t5, sizeof(t5)/sizeof(t5[0]), 8)); // Expected 8

    return 0;
}