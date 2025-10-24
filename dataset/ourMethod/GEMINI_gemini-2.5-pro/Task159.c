#include <stdio.h>
#include <stdlib.h>

/**
 * Safely reads a character from the buffer at the given index.
 *
 * @param buffer Pointer to the start of the buffer.
 * @param buffer_size The total size of the buffer.
 * @param index The index to read from.
 * @param out_value A pointer to a char where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., index out of bounds).
 */
int readFromBuffer(const char* buffer, size_t buffer_size, size_t index, char* out_value) {
    // Rule#8: Check for NULL pointers
    if (buffer == NULL || out_value == NULL) {
        return -1;
    }
    // Rules#1, Rules#4: Ensure that the index is within the buffer's boundaries.
    // Since size_t is unsigned, a check for index >= 0 is implicit.
    if (index < buffer_size) {
        *out_value = buffer[index];
        return 0; // Success
    }
    return -1; // Failure: index out of bounds
}

int main() {
    size_t buffer_size = 20;
    char* buffer = (char*)malloc(buffer_size * sizeof(char));

    // Rule#8: Check malloc's return value
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    // Populate buffer with some data
    for (size_t i = 0; i < buffer_size; ++i) {
        buffer[i] = (char)(i * 2);
    }

    printf("Testing reads from a buffer of size %zu:\n", buffer_size);
    
    // Note: In C, -1 cast to size_t becomes a very large positive number.
    size_t test_indices[] = {0, 10, 19, (size_t)-1, 20};
    int num_tests = sizeof(test_indices) / sizeof(test_indices[0]);

    for (int i = 0; i < num_tests; ++i) {
        size_t index = test_indices[i];
        printf("Test Case %d: Reading at index %zu... ", i + 1, index);
        
        char value;
        if (readFromBuffer(buffer, buffer_size, index, &value) == 0) {
            printf("Success! Value: %d\n", value);
        } else {
            printf("Failed! Index is out of bounds.\n");
        }
    }
    
    // Rule#8: Free allocated memory
    free(buffer);
    buffer = NULL;

    return 0;
}