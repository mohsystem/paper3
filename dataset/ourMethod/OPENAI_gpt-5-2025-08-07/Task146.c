#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int findMissingNumber(const int* arr, size_t len, int n) {
    if (n < 1 || arr == NULL || len != (size_t)(n - 1)) {
        return 0;
    }
    bool* seen = (bool*)calloc((size_t)(n + 1), sizeof(bool)); // index 0 unused
    if (seen == NULL) {
        return 0; // allocation failure -> treat as invalid
    }
    int xorArr = 0;
    for (size_t i = 0; i < len; ++i) {
        int v = arr[i];
        if (v < 1 || v > n) {
            free(seen);
            return 0;
        }
        if (seen[(size_t)v]) {
            free(seen);
            return 0; // duplicate
        }
        seen[(size_t)v] = true;
        xorArr ^= v;
    }
    int xorAll = 0;
    for (int i = 1; i <= n; ++i) {
        xorAll ^= i;
    }
    int missing = xorAll ^ xorArr;
    if (missing < 1 || missing > n || seen[(size_t)missing]) {
        free(seen);
        return 0;
    }
    free(seen);
    return missing;
}

int main(void) {
    // Test case 1
    int a1[] = {1, 2, 3, 5};
    printf("Missing (n=5): %d\n", findMissingNumber(a1, sizeof(a1)/sizeof(a1[0]), 5)); // 4

    // Test case 2
    int a2[] = {};
    printf("Missing (n=1): %d\n", findMissingNumber(a2, 0, 1)); // 1

    // Test case 3
    int a3[] = {2, 3};
    printf("Missing (n=3): %d\n", findMissingNumber(a3, sizeof(a3)/sizeof(a3[0]), 3)); // 1

    // Test case 4
    int a4[] = {1, 2, 3, 4, 5, 6, 8, 9, 10};
    printf("Missing (n=10): %d\n", findMissingNumber(a4, sizeof(a4)/sizeof(a4[0]), 10)); // 7

    // Test case 5 (invalid: length mismatch)
    int a5[] = {1, 2, 3, 4, 5};
    printf("Missing (invalid n=5, len=5): %d\n", findMissingNumber(a5, sizeof(a5)/sizeof(a5[0]), 5)); // 0

    return 0;
}