#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocates two chunks of memory, copies the content from the first to 
 * the second, and prints the content of the second chunk.
 * This function demonstrates safe manual memory management in C.
 *
 * @param initialData The string data to be used for the memory chunks.
 */
void manageAndCopyMemory(const char* initialData) {
    char* source = NULL;
    char* destination = NULL;
    size_t data_len = 0;

    if (initialData == NULL) {
        fprintf(stderr, "Error: Initial data cannot be null.\n");
        return;
    }
    
    // Determine size needed, including null terminator.
    data_len = strlen(initialData) + 1;

    // 1. Allocate the first chunk of memory (source).
    source = (char*)malloc(data_len);
    if (source == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for source.\n");
        goto cleanup;
    }

    // Populate the source buffer safely.
    memcpy(source, initialData, data_len);
    
    // 2. Allocate the second chunk of memory (destination).
    destination = (char*)malloc(data_len);
    if (destination == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for destination.\n");
        goto cleanup;
    }

    // 3. Copy the contents of the first chunk into the second chunk.
    // memcpy is used for a fixed-size, safe copy.
    memcpy(destination, source, data_len);

    // 4. Print the contents of the second chunk.
    printf("Copied data: \"%s\"\n", destination);

cleanup:
    // Free all allocated resources. free() is safe to call on NULL.
    free(destination);
    free(source);
}

int main() {
    printf("--- Test Case 1: Standard string ---\n");
    manageAndCopyMemory("Hello, World!");
    
    printf("\n--- Test Case 2: Empty string ---\n");
    manageAndCopyMemory("");

    printf("\n--- Test Case 3: String with numbers and symbols ---\n");
    manageAndCopyMemory("123!@#$%^&*()_+");

    printf("\n--- Test Case 4: Long string ---\n");
    manageAndCopyMemory("This is a much longer string to test the memory allocation and copy for a slightly larger chunk of data.");

    printf("\n--- Test Case 5: Null input ---\n");
    manageAndCopyMemory(NULL);

    return 0;
}