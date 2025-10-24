#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * Finds the missing number in an array containing 'size' unique integers from 1 to n,
 * where n = size + 1.
 * This function is secure against integer overflow for the sum calculation by using 'long long'.
 *
 * @param arr The input array of unique integers. It is expected to contain
 *            unique numbers from 1 to n (inclusive) with one number missing.
 * @param size The number of elements in the array.
 * @return The missing integer. Returns -1 if 'arr' is NULL and 'size' is greater than 0,
 *         indicating an invalid input state.
 */
int findMissingNumber(const int arr[], size_t size) {
    // Secure coding practice: check for invalid input (NULL pointer with non-zero size).
    if (arr == NULL && size > 0) {
        return -1; // Indicate error
    }

    // The complete list should contain 'n' numbers.
    size_t n = size + 1;

    // Use 'long long' for the expected sum to prevent potential integer overflow if 'n' is large.
    long long expectedSum = (long long)n * (n + 1) / 2;
    
    long long actualSum = 0;
    for (size_t i = 0; i < size; ++i) {
        actualSum += arr[i];
    }
    
    // The difference is the missing number.
    return (int)(expectedSum - actualSum);
}

// Helper function to run and print test cases
void run_test_case(const char* name, const int arr[], size_t size) {
    printf("%s: Array = {", name);
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("}, Missing number: %d\n", findMissingNumber(arr, size));
}

int main() {
    // Test Case 1: A standard case with a missing number in the middle.
    int arr1[] = {1, 2, 4, 5};
    run_test_case("Test Case 1", arr1, sizeof(arr1) / sizeof(arr1[0]));

    // Test Case 2: An unordered array.
    int arr2[] = {5, 2, 1, 4};
    run_test_case("Test Case 2", arr2, sizeof(arr2) / sizeof(arr2[0]));
    
    // Test Case 3: A small array where the last number is missing.
    int arr3[] = {1};
    run_test_case("Test Case 3", arr3, sizeof(arr3) / sizeof(arr3[0]));

    // Test Case 4: An empty array. Here n=1, so the missing number is 1.
    // The most portable way to represent an empty array test case in C.
    run_test_case("Test Case 4", NULL, 0);

    // Test Case 5: A larger array.
    int arr5[] = {1, 2, 3, 4, 5, 6, 7, 9, 10};
    run_test_case("Test Case 5", arr5, sizeof(arr5) / sizeof(arr5[0]));

    return 0;
}