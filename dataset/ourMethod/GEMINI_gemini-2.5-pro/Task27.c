#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Finds the outlier in an array of integers.
 * 
 * The array is guaranteed to have a length of at least 3 and is either
 * entirely comprised of odd integers or entirely comprised of even integers
 * except for a single integer N. This function finds and returns N.
 * 
 * @param arr A pointer to a constant integer array.
 * @param size The number of elements in the array.
 * @return The outlier integer N.
 */
int findOutlier(const int* arr, size_t size) {
    // Determine the majority parity by checking the first three elements.
    int even_count = 0;
    if (arr[0] % 2 == 0) {
        even_count++;
    }
    if (arr[1] % 2 == 0) {
        even_count++;
    }
    if (arr[2] % 2 == 0) {
        even_count++;
    }
    
    bool majority_is_even = even_count >= 2;

    // Iterate through the array to find the number with the opposite parity.
    for (size_t i = 0; i < size; ++i) {
        bool current_is_even = (arr[i] % 2 == 0);
        if (current_is_even != majority_is_even) {
            return arr[i];
        }
    }

    // This part should be unreachable given the problem constraints.
    // Return 0 as a fallback.
    return 0;
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

    // Test Case 3: Outlier is odd, at the end
    int test3[] = {2, 6, 8, -10, 3};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3 Outlier: %d\n", findOutlier(test3, size3));

    // Test Case 4: Outlier is even, at the end
    int test4[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 20};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4 Outlier: %d\n", findOutlier(test4, size4));

    // Test Case 5: Outlier is even (zero), at the end
    int test5[] = {1, 3, 5, 7, 9, 11, 0};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5 Outlier: %d\n", findOutlier(test5, size5));

    return 0;
}