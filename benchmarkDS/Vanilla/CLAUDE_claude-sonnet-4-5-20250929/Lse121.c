
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrate_memory_copy() {
    // Allocate two chunks of memory
    int size = 10;
    char* chunk1 = (char*)malloc(size * sizeof(char));
    char* chunk2 = (char*)malloc(size * sizeof(char));
    
    // Fill first chunk with some data
    for (int i = 0; i < size; i++) {
        chunk1[i] = 'A' + i;
    }
    
    // Copy contents of first chunk into second chunk
    memcpy(chunk2, chunk1, size);
    
    // Print contents of second chunk
    printf("Contents of chunk2 after copy:\\n");
    for (int i = 0; i < size; i++) {
        printf("%c ", chunk2[i]);
    }
    printf("\\n");
    
    // Free memory allocated to first chunk
    free(chunk1);
    chunk1 = NULL;
    
    // Print contents of second chunk again
    printf("Contents of chunk2 after freeing chunk1:\\n");
    for (int i = 0; i < size; i++) {
        printf("%c ", chunk2[i]);
    }
    printf("\\n");
    
    // Free memory allocated to second chunk
    free(chunk2);
    chunk2 = NULL;
    
    printf("Both chunks freed.\\n");
}

int main() {
    printf("Test Case 1:\\n");
    demonstrate_memory_copy();
    
    printf("\\nTest Case 2:\\n");
    demonstrate_memory_copy();
    
    printf("\\nTest Case 3:\\n");
    demonstrate_memory_copy();
    
    printf("\\nTest Case 4:\\n");
    demonstrate_memory_copy();
    
    printf("\\nTest Case 5:\\n");
    demonstrate_memory_copy();
    
    return 0;
}
