
#include <stdio.h>

#define ARRAY_SIZE 10

static const int dataArray[ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int getValueAtIndex(int index) {
    // Input validation - check if index is within valid bounds
    if (index < 0 || index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds. Valid range: 0-%d\\n", ARRAY_SIZE - 1);
        return -1; // Return error value
    }
    return dataArray[index];
}

int main() {
    // Test case 1: Valid index at beginning
    printf("Test 1 (index 0): %d\\n", getValueAtIndex(0));
    
    // Test case 2: Valid index in middle
    printf("Test 2 (index 5): %d\\n", getValueAtIndex(5));
    
    // Test case 3: Valid index at end
    printf("Test 3 (index 9): %d\\n", getValueAtIndex(9));
    
    // Test case 4: Invalid negative index
    printf("Test 4 (index -1): %d\\n", getValueAtIndex(-1));
    
    // Test case 5: Invalid index beyond array
    printf("Test 5 (index 15): %d\\n", getValueAtIndex(15));
    
    return 0;
}
