#include <stdio.h>

// A predefined constant array of IDs.
const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Retrieves the value from id_sequence at a specified index.
 *
 * @param index The index of the element to retrieve.
 * @param arraySize The total number of elements in the array.
 * @param result A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (index out of bounds).
 */
int getValueAtIndex(int index, size_t arraySize, int* result) {
    // Security Check: Validate that the index is within the bounds of the array.
    if (index >= 0 && index < arraySize) {
        *result = id_sequence[index];
        return 0; // Success
    } else {
        return -1; // Failure
    }
}

int main() {
    // Calculate the size of the array at compile time.
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // 5 Test Cases
    int testIndexes[] = {3, 0, 9, -1, 10};

    for (int i = 0; i < 5; ++i) {
        int indexToTest = testIndexes[i];
        int value;
        
        printf("Test Case %d: Testing index %d\n", i + 1, indexToTest);

        if (getValueAtIndex(indexToTest, size, &value) == 0) {
            printf(" -> Success: Value at index %d is %d\n", indexToTest, value);
        } else {
            printf(" -> Error: Index %d is out of bounds.\n", indexToTest);
        }
        printf("--------------------\n");
    }

    return 0;
}