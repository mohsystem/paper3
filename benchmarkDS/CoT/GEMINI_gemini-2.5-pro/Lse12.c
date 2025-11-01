#include <stdio.h>
#include <stdlib.h>

/**
 * Gets the value from an array at a specified index.
 * This function uses a pointer to return the result and an integer status code
 * to indicate success or failure, which is a safe pattern in C.
 *
 * @param arr The source array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @param result A pointer to an integer where the result will be stored on success.
 * @return 1 on success, 0 on failure (e.g., index out of bounds).
 */
int getValueFromArray(const int* arr, int size, int index, int* result) {
    // Security check: Ensure the index is within the valid bounds of the array.
    if (index < 0 || index >= size) {
        return 0; // Failure
    }
    *result = arr[index];
    return 1; // Success
}

int main() {
    int testArray[] = {10, 20, 30, 40, 50};
    int size = sizeof(testArray) / sizeof(testArray[0]);

    // Test cases
    int testIndexes[] = {2, 0, 4, -1, 5};
    
    printf("Running C Test Cases...\n");
    for (int i = 0; i < 5; ++i) {
        int indexToTest = testIndexes[i];
        printf("Test Case %d: Trying to get value at index %d\n", i + 1, indexToTest);
        
        int value;
        if (getValueFromArray(testArray, size, indexToTest, &value)) {
            printf("  Success: Value is %d\n", value);
        } else {
            fprintf(stderr, "  Error: Index %d is out of bounds for array size %d\n", indexToTest, size);
        }
    }

    return 0;
}