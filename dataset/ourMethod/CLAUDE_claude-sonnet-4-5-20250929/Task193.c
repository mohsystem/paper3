
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

// Function to determine if Alice wins the XOR game
// Returns true if Alice wins, false otherwise
bool xorGame(int* nums, int numsSize) {
    // Input validation: check for NULL pointer
    if (nums == NULL) {
        return false; // Fail closed on NULL input
    }
    
    // Validate array size per constraints: 1 <= nums.length <= 1000
    if (numsSize < 1 || numsSize > 1000) {
        return false; // Fail closed on invalid size
    }
    
    // Validate each element: 0 <= nums[i] < 2^16 (65536)
    // Use size_t for loop counter to avoid signed/unsigned issues
    for (size_t i = 0; i < (size_t)numsSize; i++) {
        if (nums[i] < 0 || nums[i] >= 65536) {
            return false; // Fail closed on out-of-range value
        }
    }
    
    // Calculate XOR of all elements
    // XOR is safe from overflow and order-independent
    int xorSum = 0;
    for (size_t i = 0; i < (size_t)numsSize; i++) {
        xorSum ^= nums[i];
    }
    
    // Game theory logic:
    // Alice wins if:
    // 1. Initial XOR is 0 (wins immediately by rule)
    // 2. Array has even length and XOR != 0 (optimal play guarantees win)
    // Safe modulo operation on validated size
    return (xorSum == 0) || (numsSize % 2 == 0);
}

int main() {
    // Test case 1: [1,1,2] - Expected: false
    int test1[] = {1, 1, 2};
    printf("Test 1: %s\\n", xorGame(test1, 3) ? "true" : "false");
    
    // Test case 2: [0,1] - Expected: true
    int test2[] = {0, 1};
    printf("Test 2: %s\\n", xorGame(test2, 2) ? "true" : "false");
    
    // Test case 3: [1,2,3] - Expected: true
    int test3[] = {1, 2, 3};
    printf("Test 3: %s\\n", xorGame(test3, 3) ? "true" : "false");
    
    // Test case 4: [1,1,1,1] - Expected: true (even length)
    int test4[] = {1, 1, 1, 1};
    printf("Test 4: %s\\n", xorGame(test4, 4) ? "true" : "false");
    
    // Test case 5: [5,5,5] - Expected: true (XOR = 5)
    int test5[] = {5, 5, 5};
    printf("Test 5: %s\\n", xorGame(test5, 3) ? "true" : "false");
    
    return 0;
}
