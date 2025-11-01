#include <stdio.h>
#include <stdlib.h>

/**
 * WARNING: This function demonstrates a dangerous double-free vulnerability (CWE-415).
 * Calling free() on the same memory pointer more than once leads to
 * undefined behavior, which can include memory corruption, security
 * vulnerabilities, or a program crash.
 * This code is for educational purposes only. DO NOT use in production.
 * A secure practice is to set the pointer to NULL after freeing the memory.
 */
void demonstrateDoubleFree(size_t buffer_size) {
    printf("--- Demonstrating Double Free with buffer_size = %zu ---\n", buffer_size);
    
    // 1. Allocate a buffer
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        // malloc(0) may return NULL or a unique pointer. If it's NULL, we report it.
        // It's still valid and safe to call free(NULL).
        if (buffer_size > 0) {
            printf("Memory allocation failed.\n");
            printf("--------------------------------------------------\n\n");
            return;
        } else {
             printf("malloc(0) returned NULL.\n");
        }
    } else {
        printf("Buffer allocated at address: %p\n", (void*)buffer);
    }

    // 2. Free the buffer in a loop (demonstrating the vulnerability)
    // The program will likely crash or exhibit undefined behavior on the second iteration.
    // Note: free(NULL) is a safe and well-defined no-op. The vulnerability occurs
    // only when freeing a non-NULL pointer twice.
    for (int i = 0; i < 2; ++i) {
        printf("Attempting to free buffer (iteration %d)...\n", i + 1);
        free(buffer);
        printf("free() call completed.\n");
    }
    
    printf("--------------------------------------------------\n\n");
}

int main() {
    // NOTE: The program will likely crash during the first test case
    // that successfully allocates memory. Subsequent test cases may not run.
    printf("--- Starting C Double-Free Demonstration ---\n");
    printf("WARNING: This program is expected to crash due to a double-free vulnerability.\n\n");
    
    demonstrateDoubleFree(16);
    demonstrateDoubleFree(32);
    demonstrateDoubleFree(64);
    demonstrateDoubleFree(0); // malloc(0) behavior is implementation-defined, but free is safe.
    demonstrateDoubleFree(128);

    printf("--- Demonstration Finished (This message will not be reached) ---\n");
    
    return 0;
}