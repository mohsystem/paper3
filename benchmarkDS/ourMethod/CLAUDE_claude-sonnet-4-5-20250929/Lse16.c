
#include <stdio.h>
#include <stdlib.h>

// Predefined array with values
static const int VALUES[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const size_t VALUES_SIZE = sizeof(VALUES) / sizeof(VALUES[0]);

/**
 * Retrieves the value at the specified index from the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve
 * @param result Pointer to store the result value
 * @return 0 on success, -1 on error
 */
int getValueAtIndex(int index, int *result) {
    // Null pointer check to prevent dereferencing NULL (CWE-476)
    if (result == NULL) {
        fprintf(stderr, "Error: NULL result pointer\\n");
        return -1;
    }
    
    // Initialize result to a safe default value
    *result = -1;
    
    // Input validation: check if index is within valid range
    // This prevents buffer overflow/underflow (CWE-129, CWE-125)
    // Check for negative index and cast to size_t only after validation
    if (index < 0 || (size_t)index >= VALUES_SIZE) {
        // Fail closed: return error without exposing internal details
        fprintf(stderr, "Error: Invalid index\\n");
        return -1;
    }
    
    // Safe array access after bounds validation
    *result = VALUES[index];
    return 0;
}

int main(void) {
    int result = 0;
    int status = 0;
    
    // Test cases with boundary and normal conditions
    status = getValueAtIndex(0, &result);
    printf("Test Case 1 (index=0): %d (status=%d)\\n", result, status);     // Expected: 10
    
    status = getValueAtIndex(5, &result);
    printf("Test Case 2 (index=5): %d (status=%d)\\n", result, status);     // Expected: 60
    
    status = getValueAtIndex(9, &result);
    printf("Test Case 3 (index=9): %d (status=%d)\\n", result, status);     // Expected: 100
    
    status = getValueAtIndex(-1, &result);
    printf("Test Case 4 (index=-1): %d (status=%d)\\n", result, status);    // Expected: -1 (error)
    
    status = getValueAtIndex(10, &result);
    printf("Test Case 5 (index=10): %d (status=%d)\\n", result, status);    // Expected: -1 (error)
    
    return 0;
}
