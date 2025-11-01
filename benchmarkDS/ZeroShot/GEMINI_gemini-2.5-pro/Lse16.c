#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Securely gets a value from an array at a given index.
 * 
 * This function checks for null pointers and out-of-bounds indices.
 * 
 * @param arr Pointer to the constant integer array.
 * @param size The number of elements in the array.
 * @param index The index to retrieve the value from.
 * @param result A pointer to an integer where the result will be stored on success.
 * @return Returns 0 on success and -1 on failure (e.g., null pointers, index out of bounds).
 */
int getValueAtIndex(const int* arr, size_t size, int index, int* result) {
    // Check for null pointers as a security measure
    if (arr == NULL || result == NULL) {
        return -1; 
    }
    
    // Perform bounds checking
    if (index < 0 || (size_t)index >= size) {
        return -1; // Error: index is out of bounds
    }
    
    *result = arr[index];
    return 0; // Success
}

int main() {
    int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    size_t data_size = sizeof(data) / sizeof(data[0]);
    
    // 5 Test cases: valid middle, valid start, valid end, invalid negative, invalid positive
    int testIndexes[] = {5, 0, 9, -1, 10};
    size_t num_tests = sizeof(testIndexes) / sizeof(testIndexes[0]);

    printf("Testing in C...\n");
    printf("Array: {10, 20, 30, 40, 50, 60, 70, 80, 90, 100}\n");

    for (size_t i = 0; i < num_tests; ++i) {
        int index = testIndexes[i];
        int value;
        if (getValueAtIndex(data, data_size, index, &value) == 0) {
            printf("Test case index %d: Success! Value is %d\n", index, value);
        } else {
            printf("Test case index %d: Error! Index is out of bounds or a null pointer was passed.\n", index);
        }
    }

    return 0;
}