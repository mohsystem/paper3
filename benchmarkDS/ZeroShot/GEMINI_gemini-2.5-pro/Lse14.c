#include <stdio.h>
#include <stddef.h> // For size_t

// The sequence of IDs.
static const int id_sequence[] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
// Calculate the number of elements in the array.
static const size_t id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Retrieves the value from id_sequence at a specified index.
 * This function is secure because it performs bounds checking. Instead of
 * returning the value directly, it returns a status code and populates an
 * output parameter with the value.
 *
 * @param index The index of the element to retrieve.
 * @param out_value A pointer to an integer where the result will be stored if the index is valid.
 * @return 0 on success, -1 on failure (if index is out of bounds).
 */
int getValueAtIndex(int index, int* out_value) {
    // Security check: Ensure the output pointer is not NULL.
    if (out_value == NULL) {
        return -1; // Indicate error
    }

    // Security check: Ensure the index is within the valid bounds of the array.
    if (index >= 0 && (size_t)index < id_sequence_size) {
        *out_value = id_sequence[index];
        return 0; // Success
    }

    // Index is out of bounds.
    return -1; // Failure
}

int main() {
    // 5 test cases
    int testIndexes[] = {0, 9, 4, -1, 10};
    int value;

    printf("Running 5 test cases...\n");
    for (int i = 0; i < 5; ++i) {
        int index = testIndexes[i];
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Attempting to get value at index: %d\n", index);

        // Call the function and check its return status.
        if (getValueAtIndex(index, &value) == 0) {
            printf("Success! Value at index %d is: %d\n", index, value);
        } else {
            fprintf(stderr, "Error: Index %d is out of bounds.\n", index);
        }
    }

    return 0;
}