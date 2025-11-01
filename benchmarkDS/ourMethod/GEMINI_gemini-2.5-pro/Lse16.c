#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

// Predefined data array
static const int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const size_t data_size = sizeof(data) / sizeof(data[0]);

/**
 * Gets the value at the specified index in a predefined array.
 * This is a safe function that checks boundaries before access.
 *
 * @param index The index of the element to retrieve.
 * @param out_value A pointer to an integer where the result will be stored.
 *                  This pointer must not be NULL.
 * @return true if the access was successful, false otherwise (e.g., out of bounds).
 */
bool getValueAtIndex(int index, int* out_value) {
    // Input validation: Ensure the output pointer is not NULL.
    if (out_value == NULL) {
        return false;
    }

    // Input validation: Check if the index is within the valid range.
    // Cast index to size_t for a safe comparison against data_size.
    // The check 'index >= 0' handles negative indices.
    if (index >= 0 && (size_t)index < data_size) {
        *out_value = data[index];
        return true;
    }

    // If the index is out of bounds, return false to indicate failure.
    return false;
}

int main() {
    // 5 Test Cases
    int testIndices[] = {0, 5, 9, -1, 10};
    int numTestCases = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int index = testIndices[i];
        int value = 0; // Initialize to a predictable value

        printf("Test Case %d: Trying to access index %d\n", i + 1, index);

        if (getValueAtIndex(index, &value)) {
            printf(" -> Success: Value at index %d is %d\n", index, value);
        } else {
            printf(" -> Failure: Index %d is out of bounds.\n", index);
        }
        printf("--------------------\n");
    }

    return 0;
}