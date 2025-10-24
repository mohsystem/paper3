
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Calculate XOR of all elements in the array
// Returns the XOR result, handles integer overflow safely
int calculateXOR(const int* nums, size_t numsSize) {
    // Validate input pointer is not NULL
    if (nums == NULL) {
        return -1;
    }
    
    int xorResult = 0;
    for (size_t i = 0; i < numsSize; i++) {
        xorResult ^= nums[i];
    }
    return xorResult;
}

// Determine if Alice wins the game
// Input validation ensures array length is within bounds
// Returns true if Alice wins, false otherwise, -1 on error
int xorGame(const int* nums, size_t numsSize) {
    // Input validation: check pointer is not NULL
    if (nums == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to xorGame\\n");
        return -1;
    }
    
    // Input validation: check array size is within constraints [1, 1000]
    if (numsSize == 0 || numsSize > 1000) {
        fprintf(stderr, "Error: Array size must be between 1 and 1000\\n");
        return -1;
    }
    
    // Validate each element is within bounds [0, 2^16)
    for (size_t i = 0; i < numsSize; i++) {
        if (nums[i] < 0 || nums[i] >= 65536) {
            fprintf(stderr, "Error: Array element at index %zu out of range\\n", i);
            return -1;
        }
    }
    
    // Calculate initial XOR of all elements
    int xorResult = calculateXOR(nums, numsSize);
    
    // Check for calculation error
    if (xorResult == -1) {
        fprintf(stderr, "Error: XOR calculation failed\\n");
        return -1;
    }
    
    // If XOR is 0 at start, Alice wins immediately
    if (xorResult == 0) {
        return 1;
    }
    
    // If array length is even, Alice wins
    // Mathematical proof: if XOR != 0 and length is even,
    // Alice can always mirror Bob's moves to avoid losing
    return (numsSize % 2 == 0) ? 1 : 0;
}

int main(void) {
    // Test case 1: [1,1,2] -> false
    int test1[] = {1, 1, 2};
    size_t test1Size = sizeof(test1) / sizeof(test1[0]);
    int result1 = xorGame(test1, test1Size);
    if (result1 != -1) {
        printf("Test 1: %s\\n", result1 ? "true" : "false");
    }
    
    // Test case 2: [0,1] -> true
    int test2[] = {0, 1};
    size_t test2Size = sizeof(test2) / sizeof(test2[0]);
    int result2 = xorGame(test2, test2Size);
    if (result2 != -1) {
        printf("Test 2: %s\\n", result2 ? "true" : "false");
    }
    
    // Test case 3: [1,2,3] -> true
    int test3[] = {1, 2, 3};
    size_t test3Size = sizeof(test3) / sizeof(test3[0]);
    int result3 = xorGame(test3, test3Size);
    if (result3 != -1) {
        printf("Test 3: %s\\n", result3 ? "true" : "false");
    }
    
    // Test case 4: [5,5,5,5] -> true (even length, XOR != 0)
    int test4[] = {5, 5, 5, 5};
    size_t test4Size = sizeof(test4) / sizeof(test4[0]);
    int result4 = xorGame(test4, test4Size);
    if (result4 != -1) {
        printf("Test 4: %s\\n", result4 ? "true" : "false");
    }
    
    // Test case 5: [1] -> true (single element)
    int test5[] = {1};
    size_t test5Size = sizeof(test5) / sizeof(test5[0]);
    int result5 = xorGame(test5, test5Size);
    if (result5 != -1) {
        printf("Test 5: %s\\n", result5 ? "true" : "false");
    }
    
    return 0;
}
