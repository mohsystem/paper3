#include <stdio.h>

/**
 * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 *
 * @param arr The input array of integers.
 * @param size The size of the array.
 * @return The maximum sum of a contiguous subarray.
 */
int maxSubArraySum(const int* arr, int size) {
    if (arr == NULL || size == 0) {
        return 0; // The sum of an empty subarray is 0.
    }

    int maxSoFar = arr[0];
    int currentMax = arr[0];

    for (int i = 1; i < size; i++) {
        // currentMax = max(arr[i], currentMax + arr[i]);
        if (arr[i] > currentMax + arr[i]) {
            currentMax = arr[i];
        } else {
            currentMax = currentMax + arr[i];
        }

        // maxSoFar = max(maxSoFar, currentMax);
        if (currentMax > maxSoFar) {
            maxSoFar = currentMax;
        }
    }
    return maxSoFar;
}

// Helper function to print an array for testing
void printArray(const int* arr, int size) {
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
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Max sum for ");
    printArray(arr1, n1);
    printf(" is: %d\n", maxSubArraySum(arr1, n1));

    // Test Case 2
    int arr2[] = {1};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Max sum for ");
    printArray(arr2, n2);
    printf(" is: %d\n", maxSubArraySum(arr2, n2));

    // Test Case 3
    int arr3[] = {5, 4, -1, 7, 8};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Max sum for ");
    printArray(arr3, n3);
    printf(" is: %d\n", maxSubArraySum(arr3, n3));

    // Test Case 4
    int arr4[] = {-5, -1, -3};
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Max sum for ");
    printArray(arr4, n4);
    printf(" is: %d\n", maxSubArraySum(arr4, n4));

    // Test Case 5
    int arr5[] = {-2, -3, 4, -1, -2, 1, 5, -3};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Max sum for ");
    printArray(arr5, n5);
    printf(" is: %d\n", maxSubArraySum(arr5, n5));

    return 0;
}