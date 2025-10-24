#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Given an array of integers, determine whether the sum of its elements is odd or even.
 * 
 * @param arr A pointer to a constant integer array.
 * @param size The number of elements in the array. An empty array (size 0) is considered as [0].
 * @return A constant character pointer to the string "even" or "odd".
 */
const char* oddOrEven(const int* arr, size_t size) {
    // Use long long for the sum to prevent potential integer overflow, a security risk.
    long long sum = 0;
    
    // An empty array (size == 0) will result in sum = 0, which is correct.
    for (size_t i = 0; i < size; ++i) {
        sum += arr[i];
    }

    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

// Helper function to print an array and run a test case
void run_test_case(const int* arr, size_t size) {
    printf("Input: [");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("], Output: %s\n", oddOrEven(arr, size));
}

int main() {
    // Test Case 1: Standard even case
    int test1[] = {0};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0]));

    // Test Case 2: Standard odd case
    int test2[] = {0, 1, 4};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0]));

    // Test Case 3: Case with negative numbers
    int test3[] = {0, -1, -5};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0]));

    // Test Case 4: Empty array
    run_test_case(NULL, 0);

    // Test Case 5: A longer list
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    run_test_case(test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}