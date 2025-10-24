#include <stdio.h>

// Helper function to find max of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * Calculates the length of the longest consecutive run in an array.
 * A consecutive run can be either increasing or decreasing.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The length of the longest consecutive run.
 */
int longestRun(const int arr[], int size) {
    if (arr == NULL || size == 0) {
        return 0;
    }
    if (size == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncreasing = 1;
    int currentDecreasing = 1;

    for (int i = 1; i < size; i++) {
        if (arr[i] == arr[i - 1] + 1) {
            // Increasing run continues
            currentIncreasing++;
            // Reset decreasing run
            currentDecreasing = 1;
        } else if (arr[i] == arr[i - 1] - 1) {
            // Decreasing run continues
            currentDecreasing++;
            // Reset increasing run
            currentIncreasing = 1;
        } else {
            // Run is broken
            currentIncreasing = 1;
            currentDecreasing = 1;
        }
        maxLength = max(maxLength, max(currentIncreasing, currentDecreasing));
    }
    return maxLength;
}

void printArray(const int arr[], int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]");
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1: ");
    printArray(arr1, size1);
    printf(" -> %d\n", longestRun(arr1, size1)); // Expected: 5

    // Test Case 2
    int arr2[] = {1, 2, 3, 10, 11, 15};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2: ");
    printArray(arr2, size2);
    printf(" -> %d\n", longestRun(arr2, size2)); // Expected: 3

    // Test Case 3
    int arr3[] = {5, 4, 2, 1};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3: ");
    printArray(arr3, size3);
    printf(" -> %d\n", longestRun(arr3, size3)); // Expected: 2

    // Test Case 4
    int arr4[] = {3, 5, 7, 10, 15};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4: ");
    printArray(arr4, size4);
    printf(" -> %d\n", longestRun(arr4, size4)); // Expected: 1
    
    // Test Case 5
    int arr5[] = {1, 0, -1, -2, -1, 0, 1, 2, 3};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5: ");
    printArray(arr5, size5);
    printf(" -> %d\n", longestRun(arr5, size5)); // Expected: 6

    return 0;
}