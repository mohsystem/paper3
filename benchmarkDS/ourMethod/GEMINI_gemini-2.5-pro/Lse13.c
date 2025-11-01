#include <stdio.h>
#include <stdlib.h>

// A predefined, constant sequence of IDs.
static const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
// Calculate the number of elements in the array at compile time.
static const size_t id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * @brief Retrieves the value at the specified index in the id_sequence array.
 * 
 * @param index The index of the element to retrieve.
 * @param result A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., index out of bounds).
 */
int getValueAtIndex(int index, int* result) {
    // Rule #8: All pointers must be initialized. Check for NULL pointer.
    if (result == NULL) {
        fprintf(stderr, "Error: Output pointer is null.\n");
        return -1;
    }

    // Rule #1, #4: Ensure that all buffer operations strictly adhere to the buffer's boundaries.
    if (index < 0 || (size_t)index >= id_sequence_size) {
        return -1; // Indicate error: index out of bounds
    }

    *result = id_sequence[index];
    return 0; // Indicate success
}

/**
 * @brief Main function to run test cases.
 * 
 * @return int Exit code. 0 on success.
 */
int main() {
    // 5 test cases for various scenarios.
    int testIndices[] = {0, 5, 9, -1, 10};
    int numTestCases = sizeof(testIndices) / sizeof(testIndices[0]);

    printf("Running %d test cases...\n", numTestCases);
    for (int i = 0; i < numTestCases; ++i) {
        int testIndex = testIndices[i];
        int value;
        
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Attempting to access index: %d\n", testIndex);

        // Rule #7: Check return value to handle errors.
        if (getValueAtIndex(testIndex, &value) == 0) {
            printf("Value at index %d: %d\n", testIndex, value);
        } else {
            fprintf(stderr, "Error: Index %d is out of bounds.\n", testIndex);
        }
    }

    return 0;
}