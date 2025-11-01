#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void secureMemoryOperations() {
    printf("--- C: Starting memory operations ---\n");

    char* chunk1 = NULL;
    char* chunk2 = NULL;

    // 1. Allocate the first chunk of memory of size 10.
    // It is good practice to check the return value of malloc.
    chunk1 = (char*)malloc(10 * sizeof(char));
    if (chunk1 == NULL) {
        perror("Failed to allocate memory for chunk1");
        return; // Exit on failure
    }
    printf("Step 1: Allocated chunk1 of size 10.\n");

    // Allocate the second chunk of memory of size 10.
    chunk2 = (char*)malloc(10 * sizeof(char));
    if (chunk2 == NULL) {
        perror("Failed to allocate memory for chunk2");
        free(chunk1); // IMPORTANT: Clean up previously allocated memory before exiting.
        chunk1 = NULL;
        return; // Exit on failure
    }
    printf("Step 2: Allocated chunk2 of size 10.\n");
    
    // Initialize chunk1 with some data for demonstration.
    memcpy(chunk1, "abcdefghi", 10); // Copies 9 chars + null terminator.
    printf("Initialized chunk1.\n");

    // 2. Copy the contents of the first chunk into the second chunk.
    // The programmer is responsible for ensuring the destination buffer is large enough.
    // In this case, both are size 10, so it is safe.
    memcpy(chunk2, chunk1, 10);
    printf("Step 3: Copied contents from chunk1 to chunk2.\n");

    // 3. Free the first chunk.
    free(chunk1);
    // Setting the pointer to NULL after freeing is a security best practice
    // to prevent use-after-free vulnerabilities.
    chunk1 = NULL;
    printf("Step 4: Freed chunk1 and set pointer to NULL.\n");

    // 4. Free the second chunk.
    free(chunk2);
    chunk2 = NULL;
    printf("Step 5: Freed chunk2 and set pointer to NULL.\n");

    printf("--- C: Finished memory operations ---\n");
}

int main() {
    for (int i = 0; i < 5; ++i) {
        printf("\nTest Case %d\n", i + 1);
        secureMemoryOperations();
    }
    return 0;
}