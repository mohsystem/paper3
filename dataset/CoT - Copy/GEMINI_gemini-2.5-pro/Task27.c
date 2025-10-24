#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Finds the outlier number in an array where all numbers are either even or odd, except for one.
 *
 * @param array The input integer array.
 * @param size The number of elements in the array (must be >= 3).
 * @return The outlier integer.
 */
int findOutlier(const int* array, size_t size) {
    // Since the array has at least 3 elements, we can determine the majority
    // parity by checking the first three elements.
    int evenCount = 0;
    if ((array[0] & 1) == 0) evenCount++;
    if ((array[1] & 1) == 0) evenCount++;
    if ((array[2] & 1) == 0) evenCount++;

    bool majorityIsEven = evenCount >= 2;

    // Iterate through the array to find the number with the opposite parity.
    for (size_t i = 0; i < size; ++i) {
        int num = array[i];
        if (majorityIsEven) {
            // If the majority is even, we are looking for the single odd number.
            if ((num & 1) != 0) {
                return num;
            }
        } else {
            // If the majority is odd, we are looking for the single even number.
            if ((num & 1) == 0) {
                return num;
            }
        }
    }
    
    // This part should be unreachable given the problem constraints.
    // It's included as a fallback.
    return 0;
}

void runTest(const char* testName, const int* arr, size_t size) {
    printf("%s: [", testName);
    for(size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("] -> %d\n", findOutlier(arr, size));
}


int main() {
    // Test Case 1: Odd outlier
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    runTest("Test Case 1", test1, sizeof(test1) / sizeof(test1[0]));

    // Test Case 2: Even outlier
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    runTest("Test Case 2", test2, sizeof(test2) / sizeof(test2[0]));

    // Test Case 3: Even outlier at the beginning
    int test3[] = {8, 1, 3, 5, 7, 9};
    runTest("Test Case 3", test3, sizeof(test3) / sizeof(test3[0]));

    // Test Case 4: Odd outlier at the end
    int test4[] = {2, 4, 6, 8, 10, 11};
    runTest("Test Case 4", test4, sizeof(test4) / sizeof(test4[0]));

    // Test Case 5: Even outlier in the middle
    int test5[] = {1, 3, 5, 2, 7, 9};
    runTest("Test Case 5", test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}