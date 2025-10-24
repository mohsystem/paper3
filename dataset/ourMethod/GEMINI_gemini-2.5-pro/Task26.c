#include <stdio.h>
#include <stddef.h>

// Function to find the integer that appears an odd number of times.
// It uses the XOR bitwise operator property where a ^ a = 0.
// All numbers appearing an even number of times will cancel out,
// leaving the one that appears an odd number of times.
int findOdd(const int* arr, size_t size) {
    int result = 0;
    for (size_t i = 0; i < size; ++i) {
        result ^= arr[i];
    }
    return result;
}

// Helper function to print an array
void print_array(const int* arr, size_t size) {
    printf("{ ");
    for (size_t i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf(" }");
}

// Helper function to run and print a test case
void run_test_case(const int* arr, size_t size) {
    printf("Input: ");
    print_array(arr, size);
    int result = findOdd(arr, size);
    printf(" -> Result: %d\n", result);
}

int main() {
    // Test Case 1
    int test1[] = {7};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0]));

    // Test Case 2
    int test2[] = {0};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0]));
    
    // Test Case 3
    int test3[] = {1, 1, 2};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0]));
    
    // Test Case 4
    int test4[] = {0, 1, 0, 1, 0};
    run_test_case(test4, sizeof(test4) / sizeof(test4[0]));

    // Test Case 5
    int test5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    run_test_case(test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}