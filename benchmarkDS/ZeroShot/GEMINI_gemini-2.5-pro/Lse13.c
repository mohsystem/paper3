#include <stdio.h>
#include <stdlib.h>

// The predefined array of IDs.
static const int id_sequence[] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
static const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * @brief Retrieves the value from id_sequence at a specified index.
 * This function is secure against array index out-of-bounds errors.
 *
 * @param index The index of the value to retrieve.
 * @param value_ptr A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (if index is out of bounds).
 */
int getValueAtIndex(int index, int* value_ptr) {
    // Secure: Check if the index is within the valid bounds of the array.
    if (index < 0 || index >= id_sequence_size) {
        // Check for null pointer to prevent crashes
        if (value_ptr != NULL) {
            *value_ptr = 0; // Set to a default value on error
        }
        return -1; // Return error code
    }
    
    // Check for null pointer before dereferencing
    if (value_ptr == NULL) {
        return -1; // Cannot store the result
    }

    *value_ptr = id_sequence[index];
    return 0; // Return success code
}

/**
 * @brief Main function with 5 test cases to demonstrate functionality.
 */
int main() {
    // Test cases: valid start, valid middle, valid end, invalid negative, invalid positive
    int testCases[] = {0, 4, 9, -1, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    int i;

    printf("--- Running C Test Cases ---\n");
    for (i = 0; i < numTestCases; ++i) {
        int index = testCases[i];
        int value;
        
        // Call the function and check its return status
        if (getValueAtIndex(index, &value) == 0) {
            printf("SUCCESS: Value at index %d is %d\n", index, value);
        } else {
            printf("FAILURE: Attempted to access index %d. Index is out of bounds.\n", index);
        }
    }

    return 0;
}