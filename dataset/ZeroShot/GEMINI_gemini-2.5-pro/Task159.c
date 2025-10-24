#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Securely reads a value from the buffer.
 * Performs explicit bounds checking to prevent out-of-bounds reads.
 *
 * @param buffer Pointer to the start of the buffer.
 * @param size The total size of the allocated buffer.
 * @param index The index from which to read.
 * @param out_value A pointer to a char where the read value will be stored.
 * @return true on success, false on failure (e.g., out-of-bounds index).
 */
bool secureRead(const char* buffer, size_t size, int index, char* out_value) {
    if (buffer == NULL || out_value == NULL) {
        return false; // Handle null pointers for safety
    }
    
    // The core security check: ensure the index is within the valid range [0, size-1].
    // A negative index will fail the `index >= 0` check.
    // Casting a valid positive `index` to size_t for comparison is safe.
    if (index >= 0 && (size_t)index < size) {
        *out_value = buffer[index];
        return true;
    }
    
    return false; // Index is out of bounds
}

int main() {
    const size_t bufferSize = 10;
    char* buffer = NULL;

    // 1. Dynamically allocate a memory buffer
    buffer = (char*)malloc(bufferSize * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    // Initialize buffer with some data
    for (size_t i = 0; i < bufferSize; ++i) {
        buffer[i] = (char)(i * 10);
    }
    
    printf("Buffer created with size: %zu\n", bufferSize);
    printf("Buffer content: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90]\n");
    printf("--- Running C Test Cases ---\n");

    // 2. Test cases
    int testIndices[] = {0, 5, 9, -1, 10};
    int numTests = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < numTests; ++i) {
        int index = testIndices[i];
        char value;
        if (secureRead(buffer, bufferSize, index, &value)) {
            printf("Reading at index %d: Success! Value = %d\n", index, value);
        } else {
            printf("Reading at index %d: Error! Index is out of bounds.\n", index);
        }
    }

    // Clean up allocated memory
    free(buffer);
    buffer = NULL;

    return 0;
}