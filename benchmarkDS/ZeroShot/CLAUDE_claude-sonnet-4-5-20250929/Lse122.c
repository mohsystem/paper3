
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocateAndCopy() {
    // Allocate first chunk of memory (size 10)
    char* chunk1 = (char*)malloc(10 * sizeof(char));
    
    if (chunk1 == NULL) {
        fprintf(stderr, "Memory allocation failed for chunk1\\n");
        return;
    }
    
    // Initialize chunk1 with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = i;
    }
    
    // Allocate second chunk of memory (size 10)
    char* chunk2 = (char*)malloc(10 * sizeof(char));
    
    if (chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed for chunk2\\n");
        free(chunk1);
        return;
    }
    
    // Copy contents from chunk1 to chunk2
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    chunk1 = NULL;
    
    // Free the second chunk
    free(chunk2);
    chunk2 = NULL;
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
