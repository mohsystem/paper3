#include <stdio.h>
#include <stdlib.h>

/**
 * Allocates a buffer of a specified size and prints its contents.
 * Uses calloc to allocate and initialize the buffer to zeros.
 *
 * @param size The size of the buffer to allocate.
 */
void allocateAndPrintBuffer(int size) {
    printf("--- Testing with buffer size: %d ---\n", size);
    if (size < 0) {
        printf("Error: Size cannot be negative.\n\n");
        return;
    }

    // Allocate a buffer and initialize its contents to zero.
    char *buffer = (char *)calloc(size, sizeof(char));

    // Check for allocation failure (for size > 0).
    // calloc(0, ...) behavior is implementation-defined, but we can treat it as an empty buffer.
    if (size > 0 && buffer == NULL) {
        printf("Error: Memory allocation failed.\n\n");
        return;
    }
    
    // Print the contents of the buffer.
    printf("Allocated buffer contents: [");
    for (int i = 0; i < size; ++i) {
        printf("%d", buffer[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n\n");

    // Free the allocated memory.
    free(buffer);
}

/**
 * Main function with 5 test cases.
 */
int main() {
    int test_sizes[] = {10, 0, 1, 8, 5};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    for (int i = 0; i < num_tests; ++i) {
        allocateAndPrintBuffer(test_sizes[i]);
    }

    return 0;
}