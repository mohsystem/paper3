#include <stdio.h>
#include <stdlib.h>

/**
 * WARNING: This code demonstrates a memory corruption vulnerability known as "double free".
 * Calling free() on the same pointer more than once leads to undefined behavior,
 * which can include crashing the program, corrupting heap data, or leading to
 * security exploits. This code is for educational purposes only and should not
 * be used in production environments.
 */
void freeBufferInLoop(int buffer_size) {
    if (buffer_size <= 0) {
        printf("Buffer size must be positive.\n");
        return;
    }

    printf("Allocating buffer of size: %d\n", buffer_size);
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }
    printf("Buffer allocated at address: %p\n", (void*)buffer);

    // This loop incorrectly frees the same memory multiple times.
    for (int i = 0; i < 2; ++i) {
        printf("Loop %d: Freeing buffer...\n", i + 1);
        // The second call to free() on the same pointer is a double free.
        free(buffer);
    }

    // The program state is now undefined and may crash here or later.
    printf("Demonstration of double free complete. Program might crash.\n\n");
}

int main() {
    printf("--- Test Case 1 ---\n");
    freeBufferInLoop(1024);

    // Depending on the system and compiler, the program may have already crashed.
    // If not, the heap is likely corrupted. Subsequent tests are unpredictable.
    printf("--- Test Case 2 ---\n");
    freeBufferInLoop(256);

    printf("--- Test Case 3 ---\n");
    freeBufferInLoop(1);

    printf("--- Test Case 4 ---\n");
    freeBufferInLoop(0);
    
    printf("--- Test Case 5 ---\n");
    freeBufferInLoop(4096);

    return 0;
}