#include <stdio.h>
#include <stddef.h>

/**
 * Finds the outlier number in an array where all numbers are either even or odd, except for one.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array (must be >= 3).
 * @return The single outlier integer.
 */
int findOutlier(const int* arr, size_t size) {
    int oddCount = 0;
    int evenCount = 0;
    int lastOdd = 0;
    int lastEven = 0;

    for (size_t i = 0; i < size; i++) {
        if (arr[i] % 2 == 0) {
            evenCount++;
            lastEven = arr[i];
        } else {
            oddCount++;
            lastOdd = arr[i];
        }
    }

    if (evenCount == 1) {
        return lastEven;
    } else {
        return lastOdd;
    }
}

int main() {
    // Test Case 1: Outlier is odd
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1 Outlier: %d\n", findOutlier(test1, size1));

    // Test Case 2: Outlier is even
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2 Outlier: %d\n", findOutlier(test2, size2));

    // Test Case 3: Small array, outlier is even
    int test3[] = {1, 1, 0, 1, 1};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3 Outlier: %d\n", findOutlier(test3, size3));

    // Test Case 4: Outlier is odd, at the end
    int test4[] = {2, 6, 8, -10, 3};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4 Outlier: %d\n", findOutlier(test4, size4));
    
    // Test Case 5: Negative numbers, outlier is even
    int test5[] = {-3, -5, -7, -9, 10};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5 Outlier: %d\n", findOutlier(test5, size5));

    return 0;
}