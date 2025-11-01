#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrateMemory(const char* data) {
    printf("--- Running test with data: \"%s\" ---\n", data);
    
    size_t size = strlen(data) + 1; // +1 for the null terminator

    // 1. Allocate two chunks of memory.
    char* chunk1 = (char*)malloc(size);
    char* chunk2 = (char*)malloc(size);

    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(chunk1);
        free(chunk2);
        return;
    }
    printf("Step 1: Two chunks allocated.\n");

    // 2. Copy the contents of the first chunk into the second chunk.
    strcpy(chunk1, data);
    strcpy(chunk2, chunk1);
    printf("Step 2: Contents of chunk1 copied to chunk2.\n");

    // 3. Print the contents of the second chunk.
    printf("Step 3: Contents of chunk2 after copy: %s\n", chunk2);

    // 4. Free the memory allocated to the first chunk.
    free(chunk1);
    chunk1 = NULL; // Good practice to nullify dangling pointers
    printf("Step 4: chunk1 has been freed.\n");

    // 5. Print the contents of the second chunk again.
    // The contents should be unchanged as chunk2 is a separate memory block.
    printf("Step 5: Contents of chunk2 after freeing chunk1: %s\n", chunk2);

    // 6. Free the memory allocated to the second chunk.
    free(chunk2);
    chunk2 = NULL;
    printf("Step 6: chunk2 has been freed.\n");
    printf("--- Test finished ---\n\n");
}

int main() {
    // 5 test cases
    demonstrateMemory("Hello World");
    demonstrateMemory("C Test Case");
    demonstrateMemory("12345");
    demonstrateMemory("Another piece of data.");
    demonstrateMemory(""); // Empty string test
    return 0;
}