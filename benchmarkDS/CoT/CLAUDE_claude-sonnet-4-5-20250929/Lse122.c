
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void memoryOperation() {
    // Allocate two chunks of memory each of size 10
    char* chunk1 = (char*)malloc(10);
    char* chunk2 = (char*)malloc(10);
    
    // Check if allocation was successful
    if (chunk1 == NULL || chunk2 == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        free(chunk1);
        free(chunk2);
        return;
    }
    
    // Initialize first chunk with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = (char)i;
    }
    
    // Copy contents from chunk1 to chunk2 safely
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    chunk1 = NULL;
    
    // Free the second chunk
    free(chunk2);
    chunk2 = NULL;
}

int main() {
    printf("Test Case 1: Basic memory operation\\n");
    memoryOperation();
    
    printf("Test Case 2: Multiple operations\\n");
    memoryOperation();
    
    printf("Test Case 3: Sequential operations\\n");
    memoryOperation();
    
    printf("Test Case 4: Memory stress test\\n");
    memoryOperation();
    
    printf("Test Case 5: Final operation\\n");
    memoryOperation();
    
    printf("All test cases completed successfully\\n");
    
    return 0;
}
