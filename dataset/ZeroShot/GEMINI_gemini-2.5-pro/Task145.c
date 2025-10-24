#include <stdio.h>
#include <stdlib.h> // for NULL

// Helper function to find the maximum of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 *
 * @param arr Pointer to the input array of integers.
 * @param size The number of elements in the array.
 * @return The sum of the maximum contiguous subarray. Returns 0 if the array is NULL or size is non-positive.
 */
int maxSubArraySum(const int* arr, int size) {
    // Secure: Handle NULL pointer and invalid size to prevent segmentation faults and undefined behavior.
    if (arr == NULL || size <= 0) {
        return 0; 
    }

    // Using int, assuming the sum will not overflow. For larger sums, long should be used.
    int maxSoFar = arr[0];
    int currentMax = arr[0];

    // Iterate from the second element
    for (int i = 1; i < size; i++) {
        // The maximum subarray ending at index i is either the element itself
        // or the element plus the maximum subarray ending at the previous index.
        currentMax = max(arr[i], currentMax + arr[i]);
        
        // Update the overall maximum sum found so far.
        maxSoFar = max(maxSoFar, currentMax);
    }
    
    return maxSoFar;
}

// Helper function to print an array
void print_array(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1
    int arr1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1: Array = ");
    print_array(arr1, size1);
    printf(", Max Subarray Sum = %d\n", maxSubArraySum(arr1, size1));

    // Test Case 2
    int arr2[] = {1, 2, 3, 4, 5};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2: Array = ");
    print_array(arr2, size2);
    printf(", Max Subarray Sum = %d\n", maxSubArraySum(arr2, size2));

    // Test Case 3
    int arr3[] = {-2, -3, -4, -1, -5};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3: Array = ");
    print_array(arr3, size3);
    printf(", Max Subarray Sum = %d\n", maxSubArraySum(arr3, size3));

    // Test Case 4
    int arr4[] = {5};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4: Array = ");
    print_array(arr4, size4);
    printf(", Max Subarray Sum = %d\n", maxSubArraySum(arr4, size4));

    // Test Case 5
    int arr5[] = {8, -19, 5, -4, 20};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5: Array = ");
    print_array(arr5, size5);
    printf(", Max Subarray Sum = %d\n", maxSubArraySum(arr5, size5));

    return 0;
}