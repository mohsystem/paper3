#include <stdio.h>
#include <string.h> // For memset

// A constant to prevent stack exhaustion from a very large allocation.
#define MAX_BUFFER_SIZE (1024 * 10) // 10 KB is a safe limit for stack allocation

/**
 * Allocates a buffer on the stack and overwrites it with 'A'.
 * This function uses a Variable Length Array (VLA), which is a standard
 * feature since C99. A check is performed to prevent allocating a buffer
 * so large it would likely cause a stack overflow crash.
 *
 * @param buffer_size The size of the buffer to allocate on the stack.
 */
void fillBuffer(int buffer_size) {
    if (buffer_size < 0) {
        fprintf(stderr, "Error: Buffer size cannot be negative.\n");
        return;
    }
    if (buffer_size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Requested buffer size %d exceeds the maximum allowed size of %d for stack allocation.\n", buffer_size, MAX_BUFFER_SIZE);
        return;
    }

    // VLA: allocated on the stack. Its size is determined at runtime.
    // This is standard in C99 and later.
    char buffer[buffer_size];

    // Securely fill the buffer. memset is efficient and will not write
    // past 'buffer_size' bytes.
    if (buffer_size > 0) {
        memset(buffer, 'A', buffer_size);
    }
    
    printf("Successfully allocated and filled a stack buffer of size %d.\n", buffer_size);
    // The buffer is automatically deallocated when the function returns.
}

int main() {
    printf("--- C Test Cases ---\n");
    fillBuffer(16);          // Test Case 1: Small valid size
    fillBuffer(0);           // Test Case 2: Zero size
    fillBuffer(1024);        // Test Case 3: Larger valid size
    fillBuffer(-1);          // Test Case 4: Invalid negative size
    fillBuffer(MAX_BUFFER_SIZE + 1); // Test Case 5: Size exceeding the defined maximum
    return 0;
}