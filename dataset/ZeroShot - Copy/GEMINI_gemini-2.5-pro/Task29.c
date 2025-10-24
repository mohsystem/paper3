#include <stdio.h>
#include <stdlib.h>

/**
 * Finds two different items in the array that sum up to the target value.
 * This implementation uses nested loops, resulting in O(n^2) time complexity.
 * It has O(1) space complexity.
 *
 * @param numbers A pointer to the array of integers.
 * @param n The size of the array.
 * @param target The target sum.
 * @return A pointer to a dynamically allocated array of two integers
 *         containing the indices. Returns NULL on failure.
 * @note The caller is responsible for freeing the memory of the returned array.
 */
int* twoSum(const int* numbers, size_t n, int target) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            // Check for potential overflow before addition, though unlikely with typical int sizes
            // and problem constraints. This is a good secure coding practice.
            // For simplicity in this context, we directly add.
            if (numbers[i] + numbers[j] == target) {
                int* result = (int*)malloc(2 * sizeof(int));
                if (result == NULL) {
                    // Handle memory allocation failure
                    perror("Failed to allocate memory for result");
                    return NULL;
                }
                result[0] = (int)i;
                result[1] = (int)j;
                return result;
            }
        }
    }
    // According to the problem description, a solution always exists.
    return NULL;
}

// Helper function to print an array
void print_array(const int* arr, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

// Helper function to sort two integers for consistent output
void sort_two_indices(int* arr) {
    if (arr[0] > arr[1]) {
        int temp = arr[0];
        arr[0] = arr[1];
        arr[1] = temp;
    }
}

void run_test_case(int case_num, int* numbers, size_t n, int target) {
    int* result = twoSum(numbers, n, target);
    printf("Test Case %d:\n", case_num);
    printf("Input: numbers = "); 
    print_array(numbers, n);
    printf(", target = %d\n", target);
    if (result != NULL) {
        sort_two_indices(result);
        printf("Output: {%d, %d}\n\n", result[0], result[1]);
        free(result); // Free the allocated memory
    } else {
        printf("Output: No solution found or memory allocation failed.\n\n");
    }
}

int main() {
    // Test Case 1
    int numbers1[] = {1, 2, 3};
    run_test_case(1, numbers1, sizeof(numbers1) / sizeof(numbers1[0]), 4);

    // Test Case 2
    int numbers2[] = {1234, 5678, 9012};
    run_test_case(2, numbers2, sizeof(numbers2) / sizeof(numbers2[0]), 14690);
    
    // Test Case 3
    int numbers3[] = {2, 2, 3};
    run_test_case(3, numbers3, sizeof(numbers3) / sizeof(numbers3[0]), 4);

    // Test Case 4
    int numbers4[] = {3, 2, 4};
    run_test_case(4, numbers4, sizeof(numbers4) / sizeof(numbers4[0]), 6);

    // Test Case 5
    int numbers5[] = {2, 7, 11, 15};
    run_test_case(5, numbers5, sizeof(numbers5) / sizeof(numbers5[0]), 9);
    
    return 0;
}