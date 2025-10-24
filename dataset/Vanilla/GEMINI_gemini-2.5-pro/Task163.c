#include <stdio.h>

/**
 * @brief Calculates the length of the longest consecutive-run in an array of numbers.
 * A consecutive-run is a list of adjacent, consecutive integers, either increasing or decreasing.
 * 
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The length of the longest consecutive run.
 */
int longestRun(const int arr[], int size) {
    if (size == 0) {
        return 0;
    }
    if (size == 1) {
        return 1;
    }

    int maxRun = 1;
    int currentRun = 1;
    // direction: 0 = undecided, 1 = increasing, -1 = decreasing
    int direction = 0;

    for (int i = 1; i < size; i++) {
        if (arr[i] == arr[i - 1] + 1) { // Increasing
            if (direction == 1) {
                currentRun++;
            } else {
                currentRun = 2;
                direction = 1;
            }
        } else if (arr[i] == arr[i - 1] - 1) { // Decreasing
            if (direction == -1) {
                currentRun++;
            } else {
                currentRun = 2;
                direction = -1;
            }
        } else { // Run broken
            currentRun = 1;
            direction = 0;
        }
        if (currentRun > maxRun) {
            maxRun = currentRun;
        }
    }
    return maxRun;
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test 1: longestRun({1, 2, 3, 5, 6, 7, 8, 9})\n");
    printf("Expected: 5, Got: %d\n", longestRun(arr1, size1));

    // Test Case 2
    int arr2[] = {1, 2, 3, 10, 11, 15};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("\nTest 2: longestRun({1, 2, 3, 10, 11, 15})\n");
    printf("Expected: 3, Got: %d\n", longestRun(arr2, size2));

    // Test Case 3
    int arr3[] = {5, 4, 2, 1};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("\nTest 3: longestRun({5, 4, 2, 1})\n");
    printf("Expected: 2, Got: %d\n", longestRun(arr3, size3));
    
    // Test Case 4
    int arr4[] = {3, 5, 7, 10, 15};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("\nTest 4: longestRun({3, 5, 7, 10, 15})\n");
    printf("Expected: 1, Got: %d\n", longestRun(arr4, size4));

    // Test Case 5
    int arr5[] = {1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("\nTest 5: longestRun({1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1})\n");
    printf("Expected: 6, Got: %d\n", longestRun(arr5, size5));

    return 0;
}