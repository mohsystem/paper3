#include <stdio.h>
#include <stdbool.h> // For bool type

// A static const array for the sequence.
static const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
// Calculate the number of elements in the array at compile time.
static const size_t id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Securely gets a value from the id_sequence array at a given index.
 * It performs bounds checking to prevent out-of-bounds memory access.
 *
 * @param index The index of the element to retrieve.
 * @param outValue A pointer to an integer where the result will be stored if successful.
 * @return true if the index is valid and the value was retrieved, false otherwise.
 */
bool getValueAtIndex(int index, int* outValue) {
    // Secure: Check if the output pointer is not NULL.
    if (outValue == NULL) {
        return false;
    }
    
    // Secure: Check if the index is within the valid bounds of the array.
    if (index >= 0 && (size_t)index < id_sequence_size) {
        *outValue = id_sequence[index];
        return true;
    } else {
        // Index is out of bounds, so we return false to indicate failure.
        return false;
    }
}

int main() {
    // 5 test cases for the getValueAtIndex function.
    int testIndexes[] = {0, 5, 9, -1, 10};

    printf("Running 5 test cases...\n");

    for (int i = 0; i < 5; ++i) {
        int index = testIndexes[i];
        printf("\n--- Test Case %d: Trying index %d ---\n", (i + 1), index);

        int value;
        // Call the function and check its return status.
        if (getValueAtIndex(index, &value)) {
            printf("Success: Value at index %d is %d\n", index, value);
        } else {
            printf("Error: Index %d is out of bounds or an internal error occurred.\n", index);
        }
    }

    return 0;
}