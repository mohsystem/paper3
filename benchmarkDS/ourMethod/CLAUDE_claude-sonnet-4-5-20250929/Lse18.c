
#include <stdio.h>
#include <stdlib.h>

// Predefined array for demonstration
static const int DATA_ARRAY[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const size_t ARRAY_SIZE = sizeof(DATA_ARRAY) / sizeof(DATA_ARRAY[0]);

/**
 * Returns the value at the specified index in the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve from the array
 * @return The value at the specified index, or -1 if invalid
 */
int getValueAtIndex(int index) {
    // Input validation: Check if index is negative
    // Negative values could cause undefined behavior or wraparound
    if (index < 0) {
        // Fail closed: return error indicator
        return -1;
    }
    
    // Input validation: Check if index is within valid bounds
    // Cast to size_t after negative check to prevent wraparound
    // This prevents buffer overflow and undefined behavior
    if ((size_t)index >= ARRAY_SIZE) {
        // Fail closed: return error indicator without exposing internal details
        return -1;
    }
    
    // Safe array access after validation
    return DATA_ARRAY[index];
}

int main(void) {
    // Test case 1: Valid index at beginning
    int result1 = getValueAtIndex(0);
    printf("Test 1 - Index 0: %d\\n", result1);
    
    // Test case 2: Valid index in middle
    int result2 = getValueAtIndex(5);
    printf("Test 2 - Index 5: %d\\n", result2);
    
    // Test case 3: Valid index at end
    int result3 = getValueAtIndex(9);
    printf("Test 3 - Index 9: %d\\n", result3);
    
    // Test case 4: Invalid negative index
    int result4 = getValueAtIndex(-1);
    printf("Test 4 - Index -1: %d\\n", result4);
    
    // Test case 5: Invalid index beyond array bounds
    int result5 = getValueAtIndex(15);
    printf("Test 5 - Index 15: %d\\n", result5);
    
    return 0;
}
