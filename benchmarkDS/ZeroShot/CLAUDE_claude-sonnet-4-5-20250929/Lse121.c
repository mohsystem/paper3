
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrateMemoryCopy() {
    // Allocate two chunks of memory
    const int size = 100;
    char* firstChunk = (char*)malloc(size * sizeof(char));
    char* secondChunk = (char*)malloc(size * sizeof(char));
    
    if (firstChunk == NULL || secondChunk == NULL) {
        fprintf(stderr, "Memory allocation failed!\\n");
        if (firstChunk) free(firstChunk);
        if (secondChunk) free(secondChunk);
        return;
    }
    
    // Initialize memory to zero
    memset(firstChunk, 0, size);
    memset(secondChunk, 0, size);
    
    // Fill first chunk with some data
    const char* data = "Hello, this is test data for memory operation!";
    strncpy(firstChunk, data, size - 1);
    firstChunk[size - 1] = '\\0';
    
    // Copy contents of first chunk into second chunk
    memcpy(secondChunk, firstChunk, size);
    
    // Print contents of second chunk
    printf("Contents of second chunk after copy:\\n");
    printf("%s\\n", secondChunk);
    
    // Free memory of first chunk
    free(firstChunk);
    firstChunk = NULL;
    
    // Print contents of second chunk again
    printf("\\nContents of second chunk after freeing first chunk:\\n");
    printf("%s\\n", secondChunk);
    
    // Free memory of second chunk
    free(secondChunk);
    secondChunk = NULL;
    
    printf("\\nMemory operations completed successfully.\\n");
}

int main() {
    printf("Test Case 1:\\n");
    demonstrateMemoryCopy();
    
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 2:\\n");
    demonstrateMemoryCopy();
    
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 3:\\n");
    demonstrateMemoryCopy();
    
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 4:\\n");
    demonstrateMemoryCopy();
    
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 5:\\n");
    demonstrateMemoryCopy();
    
    return 0;
}
