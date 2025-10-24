#include <stdio.h>
#include <stdlib.h>

/**
 * Reads data from a buffer at a given index.
 *
 * @param buffer The dynamically allocated buffer.
 * @param size The size of the buffer.
 * @param index The index from which to read data.
 * @param out_value A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., out-of-bounds index).
 */
int readFromBuffer(const int* buffer, size_t size, int index, int* out_value) {
    // Security: Check for null pointers and if the index is within valid bounds.
    // This prevents segmentation faults and reading from invalid memory.
    if (buffer == NULL || out_value == NULL || index < 0 || (size_t)index >= size) {
        return -1; // Indicate failure
    }
    
    *out_value = buffer[index];
    return 0; // Indicate success
}

int main() {
    const size_t BUFFER_SIZE = 10;
    int* buffer = NULL;

    // 1. Dynamically allocate a memory buffer.
    buffer = (int*)malloc(BUFFER_SIZE * sizeof(int));

    // Security: Check if malloc succeeded.
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    // 2. Fill the buffer with some data.
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = i * 10; // e.g., 0, 10, 20, ...
    }
    
    printf("C Test Cases:\n");

    // 3. Test cases to read from the buffer.
    int testIndices[] = {3, 0, 9, 10, -1};
    
    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        printf("Test %d: Reading at index %d... ", i + 1, index);

        int value;
        // Call the function and check its return status.
        if (readFromBuffer(buffer, BUFFER_SIZE, index, &value) == 0) {
            printf("Success! Value: %d\n", value);
        } else {
            printf("Error: Index is out of bounds.\n");
        }
    }
    
    // Security: Deallocate the memory to prevent memory leaks.
    free(buffer);
    buffer = NULL; // Good practice to nullify pointer after freeing.

    return 0;
}