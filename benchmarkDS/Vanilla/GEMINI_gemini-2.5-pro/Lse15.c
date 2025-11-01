#include <stdio.h>

// The predefined array
const int id_sequence[] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Gets the value at a given index from the id_sequence.
 * The prompt for user input is handled in main for testing purposes.
 * This function directly takes the index as a parameter.
 * @param index The index to retrieve the value from.
 * @param value_ptr A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (index out of bounds).
 */
int getValueAtIndex(int index, int* value_ptr) {
    if (index < 0 || index >= id_sequence_size) {
        return -1; // Error code for out of bounds
    }
    *value_ptr = id_sequence[index];
    return 0; // Success code
}

int main() {
    // 5 test cases
    int testIndices[] = {0, 4, 9, -1, 10};
    int i;

    for (i = 0; i < 5; ++i) {
        int index = testIndices[i];
        int value;
        printf("Test Case %d: Getting value at index %d\n", i + 1, index);

        if (getValueAtIndex(index, &value) == 0) {
            printf("Value: %d\n", value);
        } else {
            printf("Error: Index %d is out of bounds.\n", index);
        }
        printf("--------------------\n");
    }

    return 0;
}