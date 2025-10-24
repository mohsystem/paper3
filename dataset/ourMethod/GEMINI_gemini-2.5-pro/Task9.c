#include <stdio.h>
#include <stddef.h> // Required for size_t

// Function to determine if the sum of elements is odd or even.
// Takes a pointer to a constant integer array and its size.
// Returns a constant character pointer to a string literal ("odd" or "even").
const char* oddOrEven(const int arr[], size_t size) {
    // The problem states to treat an empty array (size == 0) as [0].
    // A loop from 0 to size-1 will not run if size is 0, leaving sum as 0, which is correct.
    long long sum = 0; // Use long long to prevent potential integer overflow.

    // Loop through the array and accumulate the sum.
    // This is bounds-checked as the loop runs from 0 to size-1.
    for (size_t i = 0; i < size; ++i) {
        sum += arr[i];
    }

    // Check the parity of the sum.
    // The modulo operator (%) correctly handles negative numbers for this parity check.
    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

int main() {
    // Test Case 1: [0] -> Expected: "even"
    int arr1[] = {0};
    size_t size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test 1: Input: [0], Output: %s\n", oddOrEven(arr1, size1));

    // Test Case 2: [0, 1, 4] -> Expected: "odd"
    int arr2[] = {0, 1, 4};
    size_t size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test 2: Input: [0, 1, 4], Output: %s\n", oddOrEven(arr2, size2));

    // Test Case 3: [0, -1, -5] -> Expected: "even"
    int arr3[] = {0, -1, -5};
    size_t size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test 3: Input: [0, -1, -5], Output: %s\n", oddOrEven(arr3, size3));

    // Test Case 4: [] -> Expected: "even" (empty array)
    int* arr4 = NULL; // Using a NULL pointer and size 0 to represent an empty array.
    size_t size4 = 0;
    printf("Test 4: Input: [], Output: %s\n", oddOrEven(arr4, size4));

    // Test Case 5: [1, 2, 3, 4, 5] -> Expected: "odd"
    int arr5[] = {1, 2, 3, 4, 5};
    size_t size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test 5: Input: [1, 2, 3, 4, 5], Output: %s\n", oddOrEven(arr5, size5));
    
    return 0;
}