
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

// Function to find the missing number in an array of unique integers from 1 to n
// Security: Uses mathematical formula to avoid overflow, validates all inputs
// Returns -1 on error, otherwise returns the missing number
int findMissingNumber(const int* arr, size_t arrSize) {
    // Rule#3: Validate input - null pointer check
    if (arr == NULL) {
        fprintf(stderr, "Error: Array pointer is NULL\\n");
        return -1;
    }
    
    // Rule#3: Validate input - array must not be empty
    if (arrSize == 0) {
        fprintf(stderr, "Error: Array cannot be empty\\n");
        return -1;
    }
    
    // Rule#3: Validate input - array size must be reasonable to prevent overflow
    // n+1 is the original size (including missing number)
    if (arrSize > 100000) {
        fprintf(stderr, "Error: Array size too large\\n");
        return -1;
    }
    
    // Rule#4: Check for potential size_t overflow when calculating n
    if (arrSize >= SIZE_MAX) {
        fprintf(stderr, "Error: Array size would cause overflow\\n");
        return -1;
    }
    
    size_t n = arrSize + 1;
    
    // Rule#4: Check for overflow in expected sum calculation
    // Maximum safe value: n * (n + 1) / 2 should not overflow long long
    if (n > (LLONG_MAX / (n + 1))) {
        fprintf(stderr, "Error: Expected sum calculation would overflow\\n");
        return -1;
    }
    
    // Calculate expected sum using formula: n * (n + 1) / 2
    long long expectedSum = ((long long)n * (n + 1)) / 2;
    long long actualSum = 0;
    
    // Rule#4: Bounds check and validate each element
    for (size_t i = 0; i < arrSize; i++) {
        // Rule#3: Validate that each number is within valid range [1, n]
        if (arr[i] < 1 || arr[i] > (int)n) {
            fprintf(stderr, "Error: Array contains invalid number %d outside range [1, %zu]\\n", 
                    arr[i], n);
            return -1;
        }
        
        // Rule#4: Check for overflow before addition
        if (actualSum > LLONG_MAX - arr[i]) {
            fprintf(stderr, "Error: Actual sum calculation would overflow\\n");
            return -1;
        }
        
        actualSum += arr[i];
    }
    
    // The missing number is the difference
    long long missing = expectedSum - actualSum;
    
    // Rule#3: Validate result is within valid range
    if (missing < 1 || missing > (long long)n) {
        fprintf(stderr, "Error: Calculated missing number %lld is invalid\\n", missing);
        return -1;
    }
    
    // Rule#3: Validate result fits in int before returning
    if (missing > INT_MAX) {
        fprintf(stderr, "Error: Missing number exceeds INT_MAX\\n");
        return -1;
    }
    
    return (int)missing;
}

int main(void) {
    int result = 0;
    
    // Test case 1: Missing number is 5
    int test1[] = {1, 2, 3, 4, 6, 7, 8, 9, 10};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    result = findMissingNumber(test1, size1);
    if (result != -1) {
        printf("Test 1 - Missing number: %d\\n", result);
    }
    
    // Test case 2: Missing number is 1
    int test2[] = {2, 3, 4, 5};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    result = findMissingNumber(test2, size2);
    if (result != -1) {
        printf("Test 2 - Missing number: %d\\n", result);
    }
    
    // Test case 3: Missing number is 10 (last element)
    int test3[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    result = findMissingNumber(test3, size3);
    if (result != -1) {
        printf("Test 3 - Missing number: %d\\n", result);
    }
    
    // Test case 4: Single element array (missing 2)
    int test4[] = {1};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    result = findMissingNumber(test4, size4);
    if (result != -1) {
        printf("Test 4 - Missing number: %d\\n", result);
    }
    
    // Test case 5: Missing number is 3 (middle element)
    int test5[] = {1, 2, 4, 5, 6};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    result = findMissingNumber(test5, size5);
    if (result != -1) {
        printf("Test 5 - Missing number: %d\\n", result);
    }
    
    return 0;
}
