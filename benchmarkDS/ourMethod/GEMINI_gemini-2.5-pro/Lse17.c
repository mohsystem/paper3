#include <stdio.h>
#include <stdlib.h>  // For size_t
#include <stdbool.h> // For bool type

/**
 * @brief Retrieves the value at a specified index in a given array.
 * This function performs bounds checking to prevent Out-of-bounds Read vulnerabilities.
 * It uses a boolean return value to indicate success or failure.
 *
 * @param arr Pointer to the constant integer array.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @param result Pointer to an integer where the result will be stored upon success.
 * @return true if the access was successful, false otherwise.
 */
bool getValueAtIndex(const int* arr, size_t size, int index, int* result) {
    // Rule #4: Validate inputs before processing. Check for NULL pointers.
    if (arr == NULL || result == NULL) {
        return false;
    }

    // Rule #1: Ensure access is within the buffer's boundaries.
    // The check `index < 0` handles negative indices.
    // The cast `(size_t)index` ensures a safe comparison with `size`.
    // If index is negative, its unsigned representation will be a very large
    // number, correctly failing the `< size` check.
    if (index < 0 || (size_t)index >= size) {
        return false;
    }

    *result = arr[index];
    return true;
}

int main() {
    // The array from which to retrieve a value.
    const int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const size_t data_size = sizeof(data) / sizeof(data[0]);

    // 5 test cases, including valid, edge, and invalid indices.
    int testIndices[] = {0, 5, 9, -1, 10};

    printf("Running test cases for array: { ");
    for(size_t i = 0; i < data_size; ++i) {
        printf("%d%s", data[i], (i == data_size - 1 ? "" : ", "));
    }
    printf(" }\n");

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        printf("\n--- Test Case %d: Attempting to access index %d ---\n", i + 1, index);
        
        int value;
        // Rule #8: Check function return status to handle errors.
        if (getValueAtIndex(data, data_size, index, &value)) {
            printf("Success: Value at index %d is %d\n", index, value);
        } else {
            fprintf(stderr, "Error: Index %d is out of bounds or an invalid argument was provided.\n", index);
        }
    }

    return 0;
}