
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024

void allocateAndFillBuffer(int bufferSize) {
    // Validate input
    if (bufferSize <= 0) {
        fprintf(stderr, "Error: Buffer size must be positive\\n");
        return;
    }
    if (bufferSize > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size exceeds maximum allowed size\\n");
        return;
    }
    
    // Allocate buffer on stack
    char buffer[MAX_BUFFER_SIZE];
    
    // Fill buffer with 'A' using safe memory operation
    memset(buffer, 'A', bufferSize);
    buffer[bufferSize] = '\\0'; // Null terminate
    
    printf("Buffer content (length %d): %.*s\\n", bufferSize, 
           bufferSize < 50 ? bufferSize : 50, buffer);
}

int main() {
    // Test case 1: Normal size
    printf("Test 1 (size 10): ");
    allocateAndFillBuffer(10);
    
    // Test case 2: Small size
    printf("Test 2 (size 1): ");
    allocateAndFillBuffer(1);
    
    // Test case 3: Medium size
    printf("Test 3 (size 50): ");
    allocateAndFillBuffer(50);
    
    // Test case 4: Larger size
    printf("Test 4 (size 500): ");
    allocateAndFillBuffer(500);
    
    // Test case 5: Edge case - invalid size
    printf("Test 5: ");
    allocateAndFillBuffer(0);
    
    return 0;
}
