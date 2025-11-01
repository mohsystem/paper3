
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocateAndCopy(const char* data) {
    if (data == NULL) {
        printf("Error: Input data is NULL\\n");
        return;
    }
    
    // Limit size to prevent excessive memory allocation
    const size_t MAX_SIZE = 1024;
    size_t dataLen = strlen(data);
    
    if (dataLen > MAX_SIZE) {
        printf("Error: Input data exceeds maximum size\\n");
        return;
    }
    
    // Allocate first chunk with bounds checking
    char* chunk1 = (char*)malloc(dataLen + 1);
    if (chunk1 == NULL) {
        printf("Error: Memory allocation failed for chunk1\\n");
        return;
    }
    
    // Allocate second chunk with same size
    char* chunk2 = (char*)malloc(dataLen + 1);
    if (chunk2 == NULL) {
        printf("Error: Memory allocation failed for chunk2\\n");
        free(chunk1);
        return;
    }
    
    // Safely copy contents using secure function
    strncpy(chunk1, data, dataLen);
    chunk1[dataLen] = '\\0';
    
    strncpy(chunk2, chunk1, dataLen);
    chunk2[dataLen] = '\\0';
    
    // Print contents of second chunk
    printf("Contents of second chunk: %s\\n", chunk2);
    
    // Clean up memory
    free(chunk1);
    free(chunk2);
}

int main() {
    // Test case 1: Normal string
    printf("Test 1:\\n");
    allocateAndCopy("Hello World");
    
    // Test case 2: Empty string
    printf("\\nTest 2:\\n");
    allocateAndCopy("");
    
    // Test case 3: String with special characters
    printf("\\nTest 3:\\n");
    allocateAndCopy("Test@123!#$");
    
    // Test case 4: NULL input
    printf("\\nTest 4:\\n");
    allocateAndCopy(NULL);
    
    // Test case 5: Longer string
    printf("\\nTest 5:\\n");
    allocateAndCopy("Secure memory allocation test");
    
    return 0;
}
