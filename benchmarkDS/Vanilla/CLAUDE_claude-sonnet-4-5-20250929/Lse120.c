
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void copyAndPrint(int size, const char* data) {
    // Allocate two chunks of memory
    char* chunk1 = (char*)malloc(size * sizeof(char));
    char* chunk2 = (char*)malloc(size * sizeof(char));
    
    // Initialize chunks with zeros
    memset(chunk1, 0, size);
    memset(chunk2, 0, size);
    
    // Copy data into first chunk
    strncpy(chunk1, data, size - 1);
    chunk1[size - 1] = '\\0';
    
    // Copy contents of first chunk into second chunk
    memcpy(chunk2, chunk1, size);
    
    // Print contents of second chunk
    printf("Contents of second chunk: %s\\n", chunk2);
    
    // Free allocated memory
    free(chunk1);
    free(chunk2);
}

int main() {
    // Test case 1
    printf("Test Case 1:\\n");
    copyAndPrint(10, "Hello");
    
    // Test case 2
    printf("\\nTest Case 2:\\n");
    copyAndPrint(15, "World");
    
    // Test case 3
    printf("\\nTest Case 3:\\n");
    copyAndPrint(20, "Java Programming");
    
    // Test case 4
    printf("\\nTest Case 4:\\n");
    copyAndPrint(5, "Test");
    
    // Test case 5
    printf("\\nTest Case 5:\\n");
    copyAndPrint(30, "Memory Allocation Example");
    
    return 0;
}
