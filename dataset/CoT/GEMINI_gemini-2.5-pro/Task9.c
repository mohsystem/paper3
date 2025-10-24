#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Given an array of integers, determines if the sum of its elements is odd or even.
 * 
 * An empty array (size 0) is treated as [0].
 * 
 * @param array Pointer to the input array of integers.
 * @param size The number of elements in the array.
 * @return const char* "even" or "odd".
 */
const char* oddOrEven(const int* array, size_t size) {
    // Use long long for the sum to prevent potential integer overflow.
    long long sum = 0;
    
    for (size_t i = 0; i < size; ++i) {
        sum += array[i];
    }
    
    // If the array is empty, sum will be 0, which is even.
    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

void run_test_case(const int* arr, size_t size) {
    printf("Input: [");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
    printf("Output: %s\n", oddOrEven(arr, size));
}

int main() {
    // Test Case 1
    int test1[] = {0};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0])); // Expected: even

    // Test Case 2
    int test2[] = {0, 1, 4};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0])); // Expected: odd

    // Test Case 3
    int test3[] = {0, -1, -5};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0])); // Expected: even

    // Test Case 4 (Empty array)
    int test4[] = {};
    run_test_case(test4, 0); // Expected: even

    // Test Case 5
    int test5[] = {1, 2, 3, 4, 5};
    run_test_case(test5, sizeof(test5) / sizeof(test5[0])); // Expected: odd

    return 0;
}