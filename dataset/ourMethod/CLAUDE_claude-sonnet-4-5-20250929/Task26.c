
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Function to find the integer that appears an odd number of times
// Uses XOR property: a^a = 0, a^0 = a, XOR is commutative and associative
// Returns the odd occurrence element, or INT_MIN on error
int findOddOccurrence(const int* arr, size_t size) {
    // Validate input: pointer must not be NULL
    if (arr == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return INT_MIN;
    }
    
    // Validate input: array must not be empty
    if (size == 0) {
        fprintf(stderr, "Error: Array size is zero\\n");
        return INT_MIN;
    }
    
    // Validate size to prevent potential overflow in loop counter
    if (size > (size_t)INT_MAX) {
        fprintf(stderr, "Error: Array size exceeds safe limits\\n");
        return INT_MIN;
    }
    
    // XOR all elements: numbers appearing even times cancel out
    // This is constant time safe and efficient O(n) time, O(1) space
    int result = 0;
    for (size_t i = 0; i < size; i++) {
        result ^= arr[i];
    }
    
    return result;
}

int main(void) {
    // Test case 1: Single element
    {
        int test1[] = {7};
        size_t size1 = sizeof(test1) / sizeof(test1[0]);
        int result1 = findOddOccurrence(test1, size1);
        if (result1 != INT_MIN) {
            printf("Test 1: %d (expected 7)\\n", result1);
        }
    }
    
    // Test case 2: Single zero
    {
        int test2[] = {0};
        size_t size2 = sizeof(test2) / sizeof(test2[0]);
        int result2 = findOddOccurrence(test2, size2);
        if (result2 != INT_MIN) {
            printf("Test 2: %d (expected 0)\\n", result2);
        }
    }
    
    // Test case 3: Simple case with pairs
    {
        int test3[] = {1, 1, 2};
        size_t size3 = sizeof(test3) / sizeof(test3[0]);
        int result3 = findOddOccurrence(test3, size3);
        if (result3 != INT_MIN) {
            printf("Test 3: %d (expected 2)\\n", result3);
        }
    }
    
    // Test case 4: Multiple occurrences
    {
        int test4[] = {0, 1, 0, 1, 0};
        size_t size4 = sizeof(test4) / sizeof(test4[0]);
        int result4 = findOddOccurrence(test4, size4);
        if (result4 != INT_MIN) {
            printf("Test 4: %d (expected 0)\\n", result4);
        }
    }
    
    // Test case 5: Complex case
    {
        int test5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
        size_t size5 = sizeof(test5) / sizeof(test5[0]);
        int result5 = findOddOccurrence(test5, size5);
        if (result5 != INT_MIN) {
            printf("Test 5: %d (expected 4)\\n", result5);
        }
    }
    
    return 0;
}
