#include <stdio.h>

// Define the sequence and its size globally
const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * @brief Returns the value at the given index in the id_sequence array.
 * 
 * @param index The index of the value to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    } else {
        return -1; // Indicate index out of bounds
    }
}

int main() {
    // 5 test cases
    int testIndices[] = {0, 3, 9, -1, 10};
    int num_tests = sizeof(testIndices) / sizeof(testIndices[0]);

    printf("Testing getValueAtIndex function:\n");
    for (int i = 0; i < num_tests; i++) {
        int index = testIndices[i];
        int value = getValueAtIndex(index);
        printf("Value at index %d: %d\n", index, value);
    }

    return 0;
}