#include <stdio.h>
#include <stdlib.h>

// A struct to hold the pair of indices for the result.
typedef struct {
    int index1;
    int index2;
} Pair;

// The function takes a pointer to an array of integers, its size, and a target integer.
// It returns a Pair struct containing the indices.
Pair two_sum(const int* numbers, size_t n, int target) {
    Pair result = {-1, -1}; // Initialize with an invalid result.
    
    // The input is guaranteed to be valid, so we don't need to check for numbers == NULL or n < 2.
    for (size_t i = 0; i < n; ++i) {
        // Start the inner loop from i + 1 to avoid using the same element twice
        // and to avoid checking the same pair in reverse order.
        for (size_t j = i + 1; j < n; ++j) {
            // If the sum of the two numbers equals the target, store their indices and return.
            if (numbers[i] + numbers[j] == target) {
                result.index1 = (int)i;
                result.index2 = (int)j;
                return result;
            }
        }
    }
    // According to the problem description, a solution always exists,
    // so this part of the code should not be reached.
    return result;
}

void run_test_case(const int* numbers, size_t n, int target) {
    Pair result = two_sum(numbers, n, target);
    printf("Input: {");
    for (size_t i = 0; i < n; ++i) {
        printf("%d%s", numbers[i], (i == n - 1 ? "" : ", "));
    }
    printf("}, Target: %d -> Result: {%d, %d}\n", target, result.index1, result.index2);
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3};
    run_test_case(arr1, sizeof(arr1) / sizeof(arr1[0]), 4);

    // Test Case 2
    int arr2[] = {1234, 5678, 9012};
    run_test_case(arr2, sizeof(arr2) / sizeof(arr2[0]), 14690);

    // Test Case 3
    int arr3[] = {2, 2, 3};
    run_test_case(arr3, sizeof(arr3) / sizeof(arr3[0]), 4);

    // Test Case 4
    int arr4[] = {3, 2, 4};
    run_test_case(arr4, sizeof(arr4) / sizeof(arr4[0]), 6);

    // Test Case 5
    int arr5[] = {2, 7, 11, 15};
    run_test_case(arr5, sizeof(arr5) / sizeof(arr5[0]), 9);

    return 0;
}