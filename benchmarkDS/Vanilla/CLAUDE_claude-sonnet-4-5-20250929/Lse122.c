
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocateAndCopy() {
    // Allocate two chunks of memory, each of size 10
    char* chunk1 = (char*)malloc(10 * sizeof(char));
    char* chunk2 = (char*)malloc(10 * sizeof(char));
    
    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed!\\n");
        return;
    }
    
    // Initialize first chunk with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = i;
    }
    
    // Copy contents from chunk1 to chunk2
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    
    // Free the second chunk
    free(chunk2);
}

int main() {
    printf("Test Case 1:\\n");
    allocateAndCopy();
    
    printf("Test Case 2:\\n");
    allocateAndCopy();
    
    printf("Test Case 3:\\n");
    allocateAndCopy();
    
    printf("Test Case 4:\\n");
    allocateAndCopy();
    
    printf("Test Case 5:\\n");
    allocateAndCopy();
    
    printf("All test cases completed successfully!\\n");
    
    return 0;
}
