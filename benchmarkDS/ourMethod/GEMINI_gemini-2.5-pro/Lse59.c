#include <stdio.h>
#include <string.h>
#include <stddef.h>

// Define a safe maximum buffer size to prevent stack overflow.
#define MAX_BUFFER_SIZE 4096

/**
 * @brief Allocates a buffer on the stack and overwrites it with 'A'.
 * 
 * Uses a Variable Length Array (VLA), a C99 feature. The buffer size
 * is validated to prevent stack exhaustion.
 * 
 * @param buffer_size The desired size of the buffer. Must be > 0 and <= MAX_BUFFER_SIZE.
 */
void overwrite_buffer(size_t buffer_size) {
    if (buffer_size == 0 || buffer_size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Invalid buffer size. Must be between 1 and %d.\n", MAX_BUFFER_SIZE);
        return;
    }

    // VLA: Allocates on the stack (C99 and later).
    char buffer[buffer_size];

    memset(buffer, 'A', buffer_size);

    printf("Successfully allocated and overwrote a buffer of size %zu.\n", buffer_size);
    // Basic check to confirm the operation
    if (buffer_size > 0 && (buffer[0] != 'A' || buffer[buffer_size - 1] != 'A')) {
        fprintf(stderr, "Buffer content verification failed.\n");
    }
}

int main(void) {
    printf("--- Test Case 1: Valid size (16) ---\n");
    overwrite_buffer(16);

    printf("\n--- Test Case 2: Edge case size (1) ---\n");
    overwrite_buffer(1);
    
    printf("\n--- Test Case 3: Maximum valid size (%d) ---\n", MAX_BUFFER_SIZE);
    overwrite_buffer(MAX_BUFFER_SIZE);

    printf("\n--- Test Case 4: Invalid size (0) ---\n");
    overwrite_buffer(0);

    printf("\n--- Test Case 5: Invalid size (too large) ---\n");
    overwrite_buffer(MAX_BUFFER_SIZE + 1);

    return 0;
}