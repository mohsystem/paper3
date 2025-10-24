
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

// Structure to hold result with max sum and subarray indices
typedef struct {
    long long max_sum;
    size_t start_index;
    size_t end_index;
} MaxSubarrayResult;

// Finds the contiguous subarray with maximum sum using Kadane's algorithm\n// Security: All inputs validated, bounds checked, overflow protection\n// Returns 0 on success, -1 on error\nint maxSubarraySum(const int* arr, size_t size, MaxSubarrayResult* result) {\n    // Input validation: NULL pointer checks\n    if (arr == NULL || result == NULL) {\n        fprintf(stderr, "Error: NULL pointer provided\\n");\n        return -1;\n    }\n    \n    // Input validation: empty array check\n    if (size == 0) {\n        fprintf(stderr, "Error: Array size cannot be zero\\n");\n        return -1;\n    }\n    \n    // Input validation: size check to prevent overflow\n    if (size > (SIZE_MAX / sizeof(int))) {\n        fprintf(stderr, "Error: Array size too large\\n");\n        return -1;\n    }\n    \n    // Initialize result structure with zeros\n    memset(result, 0, sizeof(MaxSubarrayResult));\n    \n    // Use long long to prevent integer overflow during sum calculations\n    long long max_sum = (long long)arr[0];\n    long long current_sum = (long long)arr[0];\n    \n    size_t start = 0;\n    size_t end = 0;\n    size_t temp_start = 0;\n    \n    // Kadane's algorithm with overflow protection
    for (size_t i = 1; i < size; i++) {
        // Bounds check (redundant but defensive)
        if (i >= size) {
            fprintf(stderr, "Error: Index out of bounds\\n");
            return -1;
        }
        
        long long arr_val = (long long)arr[i];
        
        // If adding current element causes negative sum, restart from current element
        if (arr_val > current_sum + arr_val) {
            current_sum = arr_val;
            temp_start = i;
        } else {
            current_sum = current_sum + arr_val;
        }
        
        // Update maximum if current sum is greater
        if (current_sum > max_sum) {
            max_sum = current_sum;
            start = temp_start;
            end = i;
        }
    }
    
    // Populate result structure
    result->max_sum = max_sum;
    result->start_index = start;
    result->end_index = end;
    
    return 0;
}

int main(void) {
    MaxSubarrayResult result;
    int return_code = 0;
    
    // Test case 1: Normal array with positive and negative numbers
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    size_t test1_size = sizeof(test1) / sizeof(test1[0]);
    return_code = maxSubarraySum(test1, test1_size, &result);
    if (return_code == 0) {
        printf("Test 1: Max sum = %lld (indices %zu to %zu)\\n", 
               result.max_sum, result.start_index, result.end_index);
    } else {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    // Test case 2: All negative numbers
    int test2[] = {-5, -2, -8, -1, -4};
    size_t test2_size = sizeof(test2) / sizeof(test2[0]);
    return_code = maxSubarraySum(test2, test2_size, &result);
    if (return_code == 0) {
        printf("Test 2: Max sum = %lld (indices %zu to %zu)\\n", 
               result.max_sum, result.start_index, result.end_index);
    } else {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    // Test case 3: All positive numbers
    int test3[] = {1, 2, 3, 4, 5};
    size_t test3_size = sizeof(test3) / sizeof(test3[0]);
    return_code = maxSubarraySum(test3, test3_size, &result);
    if (return_code == 0) {
        printf("Test 3: Max sum = %lld (indices %zu to %zu)\\n", 
               result.max_sum, result.start_index, result.end_index);
    } else {
        fprintf(stderr, "Test 3 failed\\n");
    }
    
    // Test case 4: Single element
    int test4[] = {42};
    size_t test4_size = sizeof(test4) / sizeof(test4[0]);
    return_code = maxSubarraySum(test4, test4_size, &result);
    if (return_code == 0) {
        printf("Test 4: Max sum = %lld (indices %zu to %zu)\\n", 
               result.max_sum, result.start_index, result.end_index);
    } else {
        fprintf(stderr, "Test 4 failed\\n");
    }
    
    // Test case 5: Mix with zeros
    int test5[] = {0, -3, 1, 1, 0, -2, 3};
    size_t test5_size = sizeof(test5) / sizeof(test5[0]);
    return_code = maxSubarraySum(test5, test5_size, &result);
    if (return_code == 0) {
        printf("Test 5: Max sum = %lld (indices %zu to %zu)\\n", 
               result.max_sum, result.start_index, result.end_index);
    } else {
        fprintf(stderr, "Test 5 failed\\n");
    }
    
    return 0;
}
