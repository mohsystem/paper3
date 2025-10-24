#include <stdio.h>
#include <stdlib.h> 
#include <stddef.h> 

// A helper function to find the maximum of two integers
static inline int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * @brief Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 * 
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The maximum sum of a contiguous subarray.
 * @note This function will print an error and exit if the input array is NULL or empty.
 */
int findMaxSubarraySum(const int* arr, size_t size) {
    if (arr == NULL || size == 0) {
        fprintf(stderr, "Error: Input array cannot be null or empty.\n");
        exit(EXIT_FAILURE);
    }

    int maxSoFar = arr[0];
    int currentMax = arr[0];

    for (size_t i = 1; i < size; ++i) {
        currentMax = max(arr[i], currentMax + arr[i]);
        maxSoFar = max(maxSoFar, currentMax);
    }

    return maxSoFar;
}

void run_test_case(const char* name, const int* arr, size_t size) {
    printf("%s: { ", name);
    if (arr != NULL) {
        for (size_t i = 0; i < size; ++i) {
            printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
        }
    }
    printf(" }\n");

    // The function exits on failure, so no need to check a return value here.
    int result = findMaxSubarraySum(arr, size);
    printf("Max sum: %d\n\n", result);
}

int main() {
    // Test Case 1: Mixed positive and negative numbers
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    run_test_case("Test Case 1", test1, size1);

    // Test Case 2: Mostly positive numbers
    int test2[] = {5, 4, -1, 7, 8};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    run_test_case("Test Case 2", test2, size2);

    // Test Case 3: All negative numbers
    int test3[] = {-2, -3, -1, -5};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    run_test_case("Test Case 3", test3, size3);

    // Test Case 4: Single element array
    int test4[] = {5};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    run_test_case("Test Case 4", test4, size4);

    // Test Case 5: Another mixed array
    int test5[] = {8, -19, 5, -4, 20};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    run_test_case("Test Case 5", test5, size5);

    return 0;
}