#include <stdio.h>

// A helper function to find the maximum of two long long integers.
long long max(long long a, long long b) {
    return (a > b) ? a : b;
}

/**
 * @brief Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 * 
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The maximum sum of a contiguous subarray. Returns 0 if the array is NULL or empty.
 */
long long findMaxSubarraySum(const int* arr, int size) {
    // Handle NULL pointer or empty array case.
    if (arr == NULL || size == 0) {
        return 0;
    }

    // Use long long to prevent integer overflow with large sums.
    long long maxSoFar = arr[0];
    long long currentMax = arr[0];

    for (int i = 1; i < size; i++) {
        // Decide whether to extend the existing subarray or start a new one.
        currentMax = max((long long)arr[i], currentMax + arr[i]);
        // Update the overall maximum sum found so far.
        maxSoFar = max(maxSoFar, currentMax);
    }

    return maxSoFar;
}

// Helper function to print an array
void printArray(const char* name, const int* arr, int size) {
    printf("%s: [", name);
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Mixed positive and negative numbers
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printArray("Array", test1, size1);
    printf("Maximum subarray sum: %lld\n\n", findMaxSubarraySum(test1, size1)); // Expected: 6

    // Test Case 2: All positive numbers
    int test2[] = {1, 2, 3, 4, 5};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printArray("Array", test2, size2);
    printf("Maximum subarray sum: %lld\n\n", findMaxSubarraySum(test2, size2)); // Expected: 15

    // Test Case 3: All negative numbers
    int test3[] = {-2, -3, -4, -1, -2, -1, -5, -3};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printArray("Array", test3, size3);
    printf("Maximum subarray sum: %lld\n\n", findMaxSubarraySum(test3, size3)); // Expected: -1
    
    // Test Case 4: Single element array
    int test4[] = {5};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printArray("Array", test4, size4);
    printf("Maximum subarray sum: %lld\n\n", findMaxSubarraySum(test4, size4)); // Expected: 5
    
    // Test Case 5: Empty array
    int* test5 = NULL;
    int size5 = 0;
    printArray("Array", test5, size5);
    printf("Maximum subarray sum: %lld\n\n", findMaxSubarraySum(test5, size5)); // Expected: 0

    return 0;
}