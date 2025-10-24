#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Given an array of integers, determines if the sum of its elements is odd or even.
 * 
 * @param arr Pointer to the integer array.
 * @param size The number of elements in the array. An empty array is treated as {0}.
 * @return const char* "even" or "odd".
 */
const char* oddOrEven(const int* arr, size_t size) {
    long long sum = 0; // Use long long to prevent potential overflow
    
    // An empty array (size == 0) will result in a sum of 0, which is even.
    for (size_t i = 0; i < size; i++) {
        sum += arr[i];
    }

    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

// Helper function to print an array and run the test
void runTest(const int* arr, size_t size) {
    printf("Input: [");
    for (size_t i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("], Output: %s\n", oddOrEven(arr, size));
}

int main() {
    // Test Case 1
    int arr1[] = {0};
    runTest(arr1, sizeof(arr1)/sizeof(arr1[0]));

    // Test Case 2
    int arr2[] = {0, 1, 4};
    runTest(arr2, sizeof(arr2)/sizeof(arr2[0]));

    // Test Case 3
    int arr3[] = {0, -1, -5};
    runTest(arr3, sizeof(arr3)/sizeof(arr3[0]));

    // Test Case 4: Empty array
    // A portable way for an empty array test
    runTest(NULL, 0);

    // Test Case 5
    int arr5[] = {1, 2, 3, 4, 5};
    runTest(arr5, sizeof(arr5)/sizeof(arr5[0]));

    return 0;
}