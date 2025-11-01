#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief This function demonstrates manual memory management in C.
 * It allocates two memory chunks, copies data, and then frees them.
 */
void manageMemory(void) {
    const size_t CHUNK_SIZE = 10;
    char *chunk1 = NULL;
    char *chunk2 = NULL;

    // 1. Allocate two chunks of memory each of size 10.
    chunk1 = (char*)malloc(CHUNK_SIZE);
    chunk2 = (char*)malloc(CHUNK_SIZE);

    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        // free(NULL) is a no-op and is safe to call.
        free(chunk1);
        free(chunk2);
        return;
    }
    printf("Allocated two chunks of size %zu\n", CHUNK_SIZE);

    // Initialize chunk1 with some data for demonstration
    for (size_t i = 0; i < CHUNK_SIZE; ++i) {
        chunk1[i] = (char)(i + 1);
    }
    printf("Initialized chunk1.\n");

    // 2. Copy the contents of the first chunk into the second chunk.
    // memcpy is safe here because we allocated both buffers to be CHUNK_SIZE.
    memcpy(chunk2, chunk1, CHUNK_SIZE);
    printf("Copied contents of chunk1 to chunk2.\n");

    // Verification step
    if (memcmp(chunk1, chunk2, CHUNK_SIZE) == 0) {
        printf("Verification successful: chunk2 is a copy of chunk1.\n");
    } else {
        printf("Verification failed: copy was not successful.\n");
    }
    
    // 3. Free the first chunk.
    // NOTE: If the data were sensitive, it should be securely zeroed out before freeing
    // using a function like memset_s or a custom volatile-based wipe.
    free(chunk1);
    chunk1 = NULL; // Best practice: prevent use-after-free vulnerabilities.
    printf("Freed chunk1.\n");

    // 4. Free the second chunk.
    free(chunk2);
    chunk2 = NULL; // Best practice: prevent use-after-free vulnerabilities.
    printf("Freed chunk2.\n");
    printf("Memory management simulation complete.\n");
}

int main(void) {
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        manageMemory();
    }
    return 0;
}