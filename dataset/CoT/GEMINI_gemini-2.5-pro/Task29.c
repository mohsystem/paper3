#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/**
 * Finds two indices of numbers in an array that add up to a target value.
 *
 * @param numbers The input array of integers.
 * @param count The number of elements in the array.
 * @param target The target sum.
 * @return A dynamically allocated array of 2 integers containing the indices.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if no solution is found or if memory allocation fails.
 */
int* two_sum(const int* numbers, size_t count, int target) {
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j) {
            if (numbers[i] + numbers[j] == target) {
                int* result = (int*)malloc(2 * sizeof(int));
                if (result == NULL) {
                    return NULL; // Memory allocation failed
                }
                result[0] = (int)i;
                result[1] = (int)j;
                return result;
            }
        }
    }
    // As per problem description, a solution always exists.
    return NULL;
}

void run_test_case(const char* name, int* nums, size_t count, int target) {
    int* result = two_sum(nums, count, target);
    if (result != NULL) {
        printf("%s: {%d, %d}\n", name, result[0], result[1]);
        free(result); // Free the allocated memory
    } else {
        printf("%s: No solution found or memory allocation failed.\n", name);
    }
}

int main() {
    // Test Case 1
    int nums1[] = {1, 2, 3};
    run_test_case("Test Case 1", nums1, 3, 4);

    // Test Case 2
    int nums2[] = {1234, 5678, 9012};
    run_test_case("Test Case 2", nums2, 3, 14690);

    // Test Case 3
    int nums3[] = {2, 2, 3};
    run_test_case("Test Case 3", nums3, 3, 4);
    
    // Test Case 4
    int nums4[] = {3, 2, 4};
    run_test_case("Test Case 4", nums4, 3, 6);

    // Test Case 5
    int nums5[] = {2, 7, 11, 15};
    run_test_case("Test Case 5", nums5, 4, 9);

    return 0;
}