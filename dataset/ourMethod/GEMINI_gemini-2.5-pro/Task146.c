#include <stdio.h>
#include <stddef.h> // For size_t
#include <stdlib.h> // For NULL

/**
 * @brief Finds the missing number in an array containing unique integers from 1 to n.
 *
 * @param arr A pointer to an array of unique integers from 1 to n with one missing.
 * @param size The number of elements in the array.
 * @return The missing number. Returns -1 for invalid input (NULL arr with size > 0).
 */
int findMissingNumber(const int* arr, size_t size) {
    // A robust function should check for invalid inputs.
    if (arr == NULL && size > 0) {
        return -1; // Indicate an error
    }
    
    // According to the problem description, the input is an array of unique
    // integers from 1 to n, with one missing. This implies size is n-1.
    // If the array is empty, it means n=1, the full sequence is {1},
    // and the missing number is 1.
    if (size == 0) {
        return 1;
    }

    // The full sequence should contain n numbers. The array has n-1.
    // So, n is array size + 1.
    // Use long long to avoid potential integer overflow for large n.
    long long n = size + 1;

    // Formula for sum of first n natural numbers: n * (n + 1) / 2
    long long expectedSum = n * (n + 1) / 2;

    long long actualSum = 0;
    // Iterate safely up to the given size.
    for (size_t i = 0; i < size; ++i) {
        actualSum += arr[i];
    }

    // The difference is the missing number. The result will fit in an int.
    return (int)(expectedSum - actualSum);
}

// Helper function to print an array for test cases
void print_array(const int* arr, size_t size) {
    printf("{");
    if (arr != NULL) {
        for (size_t i = 0; i < size; ++i) {
            printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
        }
    }
    printf("}");
}

void run_test(const char* test_name, const int* arr, size_t size) {
    printf("%s: Array=", test_name);
    print_array(arr, size);
    printf(", Missing=%d\n", findMissingNumber(arr, size));
}

int main() {
    // 5 test cases
    int test1[] = {1, 2, 4, 5};
    run_test("Test 1", test1, sizeof(test1) / sizeof(test1[0]));

    int test2[] = {2, 3, 1, 5};
    run_test("Test 2", test2, sizeof(test2) / sizeof(test2[0]));
    
    int test3[] = {1};
    run_test("Test 3", test3, sizeof(test3) / sizeof(test3[0]));
    
    int* test4 = NULL; // Represents an empty array
    run_test("Test 4", test4, 0);
    
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 9, 10};
    run_test("Test 5", test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}