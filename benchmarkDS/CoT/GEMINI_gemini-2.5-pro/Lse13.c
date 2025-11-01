#include <stdio.h>
#include <limits.h> // For INT_MIN

// A predefined sequence of IDs.
const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * @brief Retrieves the value from id_sequence at a specified index.
 *
 * @param index The index of the value to retrieve.
 * @return The integer value at the given index. If the index is invalid,
 *         prints an error and returns INT_MIN as an error indicator.
 */
int getValueAtIndex(int index) {
    // Security: Check if the index is within the valid bounds of the array.
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    } else {
        // If the index is invalid, return a special value to indicate an error.
        fprintf(stderr, "Error: Index %d is out of bounds for array of size %d.\n", index, id_sequence_size);
        return INT_MIN; // Error indicator
    }
}

int main() {
    // 5 test cases for the getValueAtIndex function.
    int testIndexes[] = {0, 4, 9, -1, 10};
    int num_tests = sizeof(testIndexes) / sizeof(testIndexes[0]);
    
    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        int index = testIndexes[i];
        int value = getValueAtIndex(index);

        if (value != INT_MIN) {
            printf("Value at index %d: %d\n", index, value);
        } else {
            // Error message is already printed by the function
            printf("Attempted to access index %d. See error message above.\n", index);
        }
    }

    return 0;
}