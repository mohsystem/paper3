#include <stdio.h>
#include <stdbool.h> // For bool type

// Define the array and its size
#define ARRAY_SIZE 10
static const int DATA[ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Safely gets the value from a predefined array at a given index.
 * 
 * @param index The index of the element to retrieve.
 * @param outValue A pointer to an integer where the result will be stored.
 * @return bool Returns true on success, false on failure (e.g., index out of bounds).
 */
bool getValueAtIndex(int index, int* outValue) {
    // Rule #8: Ensure output pointer is not null before use.
    if (outValue == NULL) {
        return false;
    }

    // Rule #1, #4: Validate the index to ensure it is within the buffer's boundaries.
    if (index < 0 || index >= ARRAY_SIZE) {
        // Rule #8: Fail closed by returning an error status.
        return false;
    }

    *outValue = DATA[index];
    return true;
}

int main() {
    // 5 test cases
    int testIndexes[] = {0, 5, 9, -1, 10};
    int numTests = sizeof(testIndexes) / sizeof(testIndexes[0]);

    for (int i = 0; i < numTests; ++i) {
        int index = testIndexes[i];
        int value;

        // Rule #8: Check the return value of the function to handle errors.
        if (getValueAtIndex(index, &value)) {
            printf("Value at index %d: %d\n", index, value);
        } else {
            fprintf(stderr, "Error: Index %d is out of bounds.\n", index);
        }
    }

    return 0;
}