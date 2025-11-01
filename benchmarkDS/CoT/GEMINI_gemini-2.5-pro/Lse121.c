#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrateMemoryManagement(const char* initialData) {
    printf("Processing data: \"%s\"\n", initialData);

    size_t dataSize = strlen(initialData) + 1; // +1 for the null terminator

    // 1. Allocate two chunks of memory.
    char* chunk1 = (char*)malloc(dataSize);
    char* chunk2 = (char*)malloc(dataSize);

    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(chunk1); // free(NULL) is safe
        free(chunk2);
        return;
    }
    
    // Initialize the first chunk
    memcpy(chunk1, initialData, dataSize);
    printf("Step 1 & 2: Allocated two chunks and initialized first chunk.\n");

    // 3. Copy the contents of the first chunk into the second chunk.
    memcpy(chunk2, chunk1, dataSize);
    printf("Step 3: Copied chunk1 to chunk2.\n");

    // 4. Print the contents of the second chunk.
    printf("Step 4: Contents of chunk2 after copy: %s\n", chunk2);

    // 5. Free the memory allocated to the first chunk.
    printf("Step 5: Freeing chunk1...\n");
    free(chunk1);
    chunk1 = NULL; // Good practice to avoid dangling pointers

    // 6. Print the contents of the second chunk again.
    // The content should remain as chunk2 is an independent copy.
    printf("Step 6: Contents of chunk2 after freeing chunk1: %s\n", chunk2);

    // 7. Free the memory allocated to the second chunk.
    printf("Step 7: Freeing chunk2...\n");
    free(chunk2);
    chunk2 = NULL;

    printf("----------------------------------------\n");
}

int main() {
    // 5 test cases
    demonstrateMemoryManagement("Test Case 1: Hello World");
    demonstrateMemoryManagement("Test Case 2: A simple string");
    demonstrateMemoryManagement("Test Case 3: Data");
    demonstrateMemoryManagement("Test Case 4: Another example with numbers 123");
    demonstrateMemoryManagement("Test Case 5: Final test!");

    return 0;
}