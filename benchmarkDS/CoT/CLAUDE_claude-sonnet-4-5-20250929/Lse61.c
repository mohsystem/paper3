
#include <stdio.h>
#include <stdlib.h>

void allocateAndFreeBuffer(int bufferSize) {
    if (bufferSize <= 0) {
        printf("Invalid buffer size\\n");
        return;
    }
    
    // Allocate buffer
    char* buffer = (char*)malloc(bufferSize);
    if (buffer == NULL) {
        printf("Memory allocation failed\\n");
        return;
    }
    printf("Buffer of size %d allocated\\n", bufferSize);
    
    // Use the buffer (example)
    for (int i = 0; i < (bufferSize < 10 ? bufferSize : 10); i++) {
        buffer[i] = i;
    }
    
    // Free buffer ONCE (not in a loop to prevent double-free vulnerability)
    free(buffer);
    buffer = NULL;  // Set to NULL to prevent use-after-free
    printf("Buffer freed safely\\n");
}

int main() {
    printf("Test Case 1: Small buffer\\n");
    allocateAndFreeBuffer(10);
    
    printf("\\nTest Case 2: Medium buffer\\n");
    allocateAndFreeBuffer(100);
    
    printf("\\nTest Case 3: Large buffer\\n");
    allocateAndFreeBuffer(1000);
    
    printf("\\nTest Case 4: Invalid buffer (zero)\\n");
    allocateAndFreeBuffer(0);
    
    printf("\\nTest Case 5: Invalid buffer (negative)\\n");
    allocateAndFreeBuffer(-10);
    
    return 0;
}
