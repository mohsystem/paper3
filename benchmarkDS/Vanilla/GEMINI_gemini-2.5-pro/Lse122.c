#include <stdio.h>
#include <stdlib.h> // For malloc and free
#include <string.h> // For memcpy

/**
 * Allocates two memory chunks using malloc, copies content with memcpy,
 * and frees the memory using free.
 */
void demonstrateMemoryOperations() {
    // 1. Allocate two chunks of memory each of size 10.
    // A char is guaranteed to be 1 byte, so we request 10 bytes.
    char* chunk1 = (char*)malloc(10 * sizeof(char));
    char* chunk2 = (char*)malloc(10 * sizeof(char));
    
    // Always check if malloc was successful.
    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        // Free any memory that was successfully allocated before exiting.
        free(chunk1);
        free(chunk2);
        return;
    }
    printf("Allocated two chunks of 10 bytes.\n");

    // For demonstration, fill the first chunk.
    for (int i = 0; i < 10; ++i) {
        chunk1[i] = (char)(i + 1);
    }
    
    // 2. Copy the contents of the first chunk into the second chunk.
    memcpy(chunk2, chunk1, 10);
    printf("Copied contents from chunk1 to chunk2.\n");
    
    // 3. Free the first chunk.
    free(chunk1);
    chunk1 = NULL; // Good practice to set pointer to NULL after freeing.
    printf("Freed the first chunk.\n");

    // 4. Free the second chunk.
    free(chunk2);
    chunk2 = NULL; // Good practice.
    printf("Freed the second chunk.\n");
}

int main() {
    printf("--- Running C Test Cases ---\n");
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        demonstrateMemoryOperations();
        printf("--- End of Test Case %d ---\n", i);
    }
    return 0;
}