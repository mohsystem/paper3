#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Finds two indices of numbers in an array that add up to a target value.
 * @note The returned array must be malloced, assume caller calls free().
 * 
 * @param numbers The input array of integers.
 * @param n The size of the input array.
 * @param target The target sum.
 * @return A pointer to a dynamically allocated array of two integers 
 *         representing the indices, or NULL if no solution is found or on error.
 */
int* two_sum(const int* numbers, int n, int target) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (numbers[i] + numbers[j] == target) {
                int* result = (int*)malloc(2 * sizeof(int));
                if (result == NULL) {
                    return NULL; // Memory allocation failed
                }
                result[0] = i;
                result[1] = j;
                return result;
            }
        }
    }
    // According to the problem description, a solution always exists.
    return NULL;
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3};
    int* result1 = two_sum(arr1, 3, 4);
    if (result1 != NULL) {
        printf("Test 1: [%d, %d]\n", result1[0], result1[1]); // Expected: [0, 2]
        free(result1);
    }

    // Test Case 2
    int arr2[] = {1234, 5678, 9012};
    int* result2 = two_sum(arr2, 3, 14690);
    if (result2 != NULL) {
        printf("Test 2: [%d, %d]\n", result2[0], result2[1]); // Expected: [1, 2]
        free(result2);
    }
    
    // Test Case 3
    int arr3[] = {2, 2, 3};
    int* result3 = two_sum(arr3, 3, 4);
    if (result3 != NULL) {
        printf("Test 3: [%d, %d]\n", result3[0], result3[1]); // Expected: [0, 1]
        free(result3);
    }
    
    // Test Case 4
    int arr4[] = {3, 2, 4};
    int* result4 = two_sum(arr4, 3, 6);
    if (result4 != NULL) {
        printf("Test 4: [%d, %d]\n", result4[0], result4[1]); // Expected: [1, 2]
        free(result4);
    }
    
    // Test Case 5
    int arr5[] = {2, 7, 11, 15};
    int* result5 = two_sum(arr5, 4, 9);
    if (result5 != NULL) {
        printf("Test 5: [%d, %d]\n", result5[0], result5[1]); // Expected: [0, 1]
        free(result5);
    }
    
    return 0;
}