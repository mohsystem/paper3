#include <stdio.h>

// The predefined array
const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Takes an index and returns the value at that index in the id_sequence array.
 * Performs bounds checking to prevent security vulnerabilities.
 *
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Security check: ensure the index is within the valid bounds of the array.
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    } else {
        // Return a sentinel value to indicate an error (index out of bounds).
        return -1;
    }
}

int main() {
    // 5 test cases
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid index in the middle
    int index1 = 3;
    printf("Test Case 1: Index = %d, Value = %d\n", index1, getValueAtIndex(index1));

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    printf("Test Case 2: Index = %d, Value = %d\n", index2, getValueAtIndex(index2));

    // Test Case 3: Valid index at the end
    int index3 = id_sequence_size - 1;
    printf("Test Case 3: Index = %d, Value = %d\n", index3, getValueAtIndex(index3));

    // Test Case 4: Invalid index (negative)
    int index4 = -1;
    printf("Test Case 4: Index = %d, Value = %d\n", index4, getValueAtIndex(index4));

    // Test Case 5: Invalid index (out of bounds)
    int index5 = id_sequence_size;
    printf("Test Case 5: Index = %d, Value = %d\n", index5, getValueAtIndex(index5));

    return 0;
}