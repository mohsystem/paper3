
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrateMemoryOperations() {
    const char* initialData = "Hello, Secure World!";
    const size_t dataSize = strlen(initialData) + 1;
    
    /* Allocate first chunk */
    char* firstChunk = (char*)malloc(dataSize);
    if (firstChunk == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for first chunk\\n");
        return;
    }
    
    /* Copy initial data to first chunk */
    strncpy(firstChunk, initialData, dataSize - 1);
    firstChunk[dataSize - 1] = '\\0';
    printf("First chunk allocated: %s\\n", firstChunk);
    
    /* Allocate second chunk */
    char* secondChunk = (char*)malloc(dataSize);
    if (secondChunk == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for second chunk\\n");
        free(firstChunk);
        return;
    }
    
    /* Copy contents from first chunk to second chunk */
    strncpy(secondChunk, firstChunk, dataSize - 1);
    secondChunk[dataSize - 1] = '\\0';
    printf("Second chunk after copy: %s\\n", secondChunk);
    
    /* Free first chunk */
    free(firstChunk);
    firstChunk = NULL;
    printf("First chunk freed\\n");
    
    /* Print second chunk after first is freed */
    printf("Second chunk after first freed: %s\\n", secondChunk);
    
    /* Free second chunk */
    free(secondChunk);
    secondChunk = NULL;
    printf("Second chunk freed\\n");
}

int main() {
    printf("=== Test Case 1: Basic operation ===\\n");
    demonstrateMemoryOperations();
    
    printf("\\n=== Test Case 2: Basic operation ===\\n");
    demonstrateMemoryOperations();
    
    printf("\\n=== Test Case 3: Basic operation ===\\n");
    demonstrateMemoryOperations();
    
    printf("\\n=== Test Case 4: Basic operation ===\\n");
    demonstrateMemoryOperations();
    
    printf("\\n=== Test Case 5: Basic operation ===\\n");
    demonstrateMemoryOperations();
    
    return 0;
}
