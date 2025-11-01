#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy, memset

// Helper function to print a memory chunk in a readable hex format.
void print_chunk_c(const char* name, const char* chunk, size_t size) {
    if (chunk == NULL) {
        printf("   %s is NULL\n", name);
        return;
    }
    printf("   Content of %s: [ ", name);
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", (unsigned char)chunk[i]);
    }
    printf("]\n");
}

void manageMemory(int testCaseNumber) {
    const size_t SIZE = 10;
    printf("--- Starting Memory Management Demo ---\n");

    // 1. Allocate two chunks of memory each of size 10.
    // Always check the return value of malloc for NULL to handle allocation failures.
    char* chunk1 = (char*)malloc(SIZE * sizeof(char));
    if (chunk1 == NULL) {
        fprintf(stderr, "Error: Failed to allocate chunk1.\n");
        return;
    }
    char* chunk2 = (char*)malloc(SIZE * sizeof(char));
    if (chunk2 == NULL) {
        fprintf(stderr, "Error: Failed to allocate chunk2.\n");
        free(chunk1); // Clean up previously allocated memory before returning.
        return;
    }
    printf("1. Allocated two memory chunks of size %zu\n", SIZE);

    // Initialize the first chunk with some data.
    // `snprintf` is used for safe string formatting to avoid buffer overflows.
    char testData[SIZE];
    snprintf(testData, SIZE, "Test %d", testCaseNumber);
    memcpy(chunk1, testData, SIZE);
    
    print_chunk_c("chunk1", chunk1, SIZE);
    memset(chunk2, 0, SIZE); // Zero out chunk2 to show it's initially empty.
    print_chunk_c("chunk2 (before copy)", chunk2, SIZE);

    // 2. Copy the contents of the first chunk into the second chunk.
    // `memcpy` is used for a raw memory copy. The size is known, so it's safe here.
    memcpy(chunk2, chunk1, SIZE);
    printf("2. Copied chunk1 to chunk2.\n");
    print_chunk_c("chunk2 (after copy)", chunk2, SIZE);

    // 3. Free the first chunk.
    free(chunk1);
    chunk1 = NULL; // Security best practice: set pointer to NULL after freeing to prevent use-after-free.
    printf("3. Freed first chunk.\n");

    // 4. Free the second chunk.
    free(chunk2);
    chunk2 = NULL; // Prevent use-after-free.
    printf("4. Freed second chunk.\n");

    printf("--- Demo Finished ---\n\n");
}

int main() {
    printf(">>> Running 5 Test Cases for Memory Management in C <<<\n");
    for (int i = 1; i <= 5; ++i) {
        printf(">>> Test Case #%d <<<\n", i);
        manageMemory(i);
    }
    return 0;
}