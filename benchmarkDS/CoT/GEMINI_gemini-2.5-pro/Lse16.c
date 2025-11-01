#include <stdio.h>
#include <limits.h> // For INT_MIN

/**
 * Returns the value at the given index from the provided array.
 *
 * @param array A constant pointer to the integer array.
 * @param size The total number of elements in the array.
 * @param index The integer index to look up.
 * @return The value at the specified index. If the index is out of bounds,
 *         it returns INT_MIN to signify an error.
 */
int getValueAtIndex(const int array[], int size, int index) {
    // Security Check: Ensure the index is within the valid bounds of the array.
    // This prevents out-of-bounds memory access.
    if (index < 0 || index >= size) {
        // Return a special value to indicate an error.
        return INT_MIN;
    }
    
    // If the index is valid, return the element at that position.
    return array[index];
}

/**
 * Main function with 5 test cases to demonstrate the functionality.
 */
int main() {
    // A pre-defined array.
    int predefined_array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    // Calculate the size of the array safely.
    int array_size = sizeof(predefined_array) / sizeof(predefined_array[0]);

    // Test cases including valid, edge, and invalid indices.
    int testCases[] = {5, 0, 9, -1, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases...\n");
    for (int i = 0; i < numTestCases; ++i) {
        int index = testCases[i];
        printf("Testing index: %d\n", index);
        
        int result = getValueAtIndex(predefined_array, array_size, index);

        // Check for the error value.
        if (result == INT_MIN) {
            printf("Error: Index is out of bounds.\n");
        } else {
            printf("Value at index %d: %d\n", index, result);
        }
        printf("---\n");
    }

    return 0;
}