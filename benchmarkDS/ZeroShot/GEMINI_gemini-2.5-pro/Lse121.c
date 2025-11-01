#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A helper function to print a memory chunk that may not be null-terminated.
void printChunk(const char* chunk, size_t size) {
    if (chunk == NULL) {
        printf("[null chunk]");
        return;
    }
    // fwrite is a safe way to write a specific number of bytes to a stream.
    fwrite(chunk, sizeof(char), size, stdout);
}

/**
 * @brief Demonstrates secure memory allocation, copying, and freeing in C.
 *
 * @param content The C-string content to use for the memory chunks.
 * @param size The number of bytes to allocate and copy.
 */
void demonstrateMemoryOperations(const char* content, size_t size) {
    printf("---- Demonstrating for content: \"");
    printChunk(content, size);
    printf("\" ----\n");
    
    // 1. Allocate two chunks of memory on the heap.
    char* chunk1 = (char*)malloc(size);
    char* chunk2 = (char*)malloc(size);

    // Secure coding: Always check if malloc returned NULL.
    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        // Clean up any partial allocation before exiting.
        free(chunk1);
        free(chunk2);
        return;
    }
    
    // Initialize chunk1 with the provided content.
    // memcpy is used for raw memory copy. It's secure if the size is correct.
    if (size > 0) {
        memcpy(chunk1, content, size);
    }
    
    // 2. Copy the contents of the first chunk into the second chunk.
    if (size > 0) {
        memcpy(chunk2, chunk1, size);
    }
    
    // 3. Print the contents of the second chunk.
    printf("Contents of chunk2 before freeing chunk1: ");
    printChunk(chunk2, size);
    printf("\n");
    
    // 4. Free the memory allocated to the first chunk.
    free(chunk1);
    chunk1 = NULL; // Good practice: prevent use of dangling pointer.
    
    printf("chunk1 has been freed.\n");
    
    // 5. Print the contents of the second chunk again.
    // The contents of chunk2 are independent and remain valid.
    printf("Contents of chunk2 after freeing chunk1:  ");
    printChunk(chunk2, size);
    printf("\n");

    // 6. Free the memory allocated to the second chunk.
    free(chunk2);
    chunk2 = NULL;
    printf("chunk2 has been freed.\n");
    printf("------------------------------------------\n\n");
}

int main() {
    // 5 test cases
    const char* test1 = "Hello, Secure World!";
    demonstrateMemoryOperations(test1, strlen(test1));

    const char* test2 = "";
    demonstrateMemoryOperations(test2, strlen(test2));

    const char test3[] = {'A', '\0', 'B', '\0', 'C'};
    demonstrateMemoryOperations(test3, sizeof(test3));

    const char* test4 = "This is a much longer string to test a slightly larger allocation of memory.";
    demonstrateMemoryOperations(test4, strlen(test4));

    const char* test5 = "!@#$%^&*()_+-=[]{}|;':,./<>?";
    demonstrateMemoryOperations(test5, strlen(test5));

    return 0;
}