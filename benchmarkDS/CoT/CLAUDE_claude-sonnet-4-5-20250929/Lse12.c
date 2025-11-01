
#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(int* array, int size, int index) {
    // Security check: validate index to prevent out-of-bounds access
    if (array == NULL || index < 0 || index >= size) {
        return -1; // Return error code for invalid access
    }
    return array[index];
}

int main() {
    // Test cases
    int testArray1[] = {10, 20, 30, 40, 50};
    int size = 5;
    
    // Test case 1: Valid index in middle
    printf("Test 1: %d\\n", getValueFromArray(testArray1, size, 2)); // Expected: 30
    
    // Test case 2: Valid index at start
    printf("Test 2: %d\\n", getValueFromArray(testArray1, size, 0)); // Expected: 10
    
    // Test case 3: Valid index at end
    printf("Test 3: %d\\n", getValueFromArray(testArray1, size, 4)); // Expected: 50
    
    // Test case 4: Invalid index (negative)
    printf("Test 4: %d\\n", getValueFromArray(testArray1, size, -1)); // Expected: -1
    
    // Test case 5: Invalid index (out of bounds)
    printf("Test 5: %d\\n", getValueFromArray(testArray1, size, 10)); // Expected: -1
    
    // Original functionality: read from standard input
    int index;
    if (scanf("%d", &index) == 1) {
        int result = getValueFromArray(testArray1, size, index);
        return (result >= 0) ? result : 1;
    }
    
    return 1;
}
