
#include <stdio.h>
#include <stdlib.h>

void allocateAndFreeBuffer(int bufferSize, int iterations) {
    for (int i = 0; i < iterations; i++) {
        char* buffer = (char*)malloc(bufferSize);
        if (buffer == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            return;
        }
        free(buffer);
    }
}

int main() {
    // Test case 1: Small buffer, few iterations
    printf("Test 1: Buffer size 1024, 5 iterations\\n");
    allocateAndFreeBuffer(1024, 5);
    
    // Test case 2: Medium buffer, moderate iterations
    printf("Test 2: Buffer size 4096, 10 iterations\\n");
    allocateAndFreeBuffer(4096, 10);
    
    // Test case 3: Large buffer, few iterations
    printf("Test 3: Buffer size 1048576, 3 iterations\\n");
    allocateAndFreeBuffer(1048576, 3);
    
    // Test case 4: Small buffer, many iterations
    printf("Test 4: Buffer size 512, 20 iterations\\n");
    allocateAndFreeBuffer(512, 20);
    
    // Test case 5: Very small buffer, moderate iterations
    printf("Test 5: Buffer size 128, 15 iterations\\n");
    allocateAndFreeBuffer(128, 15);
    
    printf("All tests completed\\n");
    
    return 0;
}
