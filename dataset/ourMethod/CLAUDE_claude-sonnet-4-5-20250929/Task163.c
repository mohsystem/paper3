
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

// Function to find the longest consecutive run in an array
// Security: Validates input size, bounds checks all array accesses, prevents overflow
int longestRun(const int* arr, size_t size) {
    // Input validation: NULL pointer check
    if (arr == NULL) {
        fprintf(stderr, "Error: Array pointer is NULL\\n");
        return -1;
    }
    
    // Input validation: empty array check
    if (size == 0) {
        fprintf(stderr, "Error: Array size is zero\\n");
        return -1;
    }
    
    // Input validation: size check to prevent potential overflow
    if (size > (size_t)INT_MAX) {
        fprintf(stderr, "Error: Array size exceeds maximum safe value\\n");
        return -1;
    }
    
    // Base case: single element returns 1
    if (size == 1) {
        return 1;
    }
    
    int maxRun = 1;  // Initialize to 1 as per problem requirements
    int currentRun = 1;
    
    // Iterate through array safely with bounds checking
    for (size_t i = 1; i < size; i++) {
        // Security: Explicit bounds check before array access
        if (i >= size) {
            break;
        }
        
        // Calculate difference safely to avoid overflow
        // Use long long for intermediate calculation
        long long current = (long long)arr[i];
        long long previous = (long long)arr[i - 1];
        long long diff = current - previous;
        
        // Check if consecutive (diff is +1 or -1)
        if (diff == 1LL || diff == -1LL) {
            // Security: Check for integer overflow before incrementing
            if (currentRun == INT_MAX) {
                fprintf(stderr, "Error: Run length exceeds maximum integer value\\n");
                return -1;
            }
            currentRun++;
        } else {
            // Update maxRun and reset currentRun
            if (currentRun > maxRun) {
                maxRun = currentRun;
            }
            currentRun = 1;
        }
    }
    
    // Final comparison to catch runs that end at array boundary
    if (currentRun > maxRun) {
        maxRun = currentRun;
    }
    
    return maxRun;
}

int main(void) {
    // Test case 1: Mixed consecutive runs
    int test1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: %d (expected: 5)\\n", longestRun(test1, size1));
    
    // Test case 2: Consecutive run at start
    int test2[] = {1, 2, 3, 10, 11, 15};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: %d (expected: 3)\\n", longestRun(test2, size2));
    
    // Test case 3: Decreasing consecutive runs
    int test3[] = {5, 4, 2, 1};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: %d (expected: 2)\\n", longestRun(test3, size3));
    
    // Test case 4: No consecutive runs
    int test4[] = {3, 5, 7, 10, 15};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: %d (expected: 1)\\n", longestRun(test4, size4));
    
    // Test case 5: Single element
    int test5[] = {42};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: %d (expected: 1)\\n", longestRun(test5, size5));
    
    return 0;
}
