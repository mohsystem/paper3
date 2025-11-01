#include <stdio.h>

// The predefined array
const int anArray[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int arraySize = sizeof(anArray) / sizeof(anArray[0]);

/**
 * Returns the value at the specified index of the predefined array.
 *
 * @param index The index of the element to return.
 * @return The element at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Check if the index is within the valid range
    if (index >= 0 && index < arraySize) {
        return anArray[index];
    } else {
        // Return a special value to indicate an invalid index
        return -1;
    }
}

int main() {
    // 5 Test Cases
    printf("Test Case 1 (index = 3): %d\n", getValueAtIndex(3));
    printf("Test Case 2 (index = 0): %d\n", getValueAtIndex(0));
    printf("Test Case 3 (index = 9): %d\n", getValueAtIndex(9));
    printf("Test Case 4 (index = 10): %d\n", getValueAtIndex(10)); // Out of bounds
    printf("Test Case 5 (index = -1): %d\n", getValueAtIndex(-1)); // Out of bounds
    
    return 0;
}