
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Demonstrates proper memory management pattern */
/* Allocates and deallocates buffer correctly - no double-free */
int allocateAndFreeBuffer(size_t bufferSize, int iterations) {
    /* Validate input to prevent negative or excessive allocations */
    if (bufferSize == 0 || bufferSize > 100 * 1024 * 1024) { /* Max 100MB */
        fprintf(stderr, "Invalid buffer size\\n");
        return -1;
    }
    if (iterations <= 0 || iterations > 1000) {
        fprintf(stderr, "Invalid iteration count\\n");
        return -1;
    }
    
    /* Correct pattern: allocate and free within each iteration */
    /* This prevents double-free vulnerability (CWE-415) */
    for (int i = 0; i < iterations; i++) {
        /* Allocate buffer for this iteration */
        unsigned char* buffer = (unsigned char*)malloc(bufferSize);
        
        /* Check allocation success - always validate malloc result */
        if (buffer == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            return -1;
        }
        
        /* Use buffer (example: initialize to demonstrate allocation worked) */
        if (bufferSize > 0) {
            buffer[0] = (unsigned char)i;
        }
        
        /* Free the buffer exactly once per allocation */
        /* This is the correct pattern - one free per malloc */
        free(buffer);
        
        /* Set pointer to NULL after free to prevent use-after-free */
        buffer = NULL;
    }
    
    return 0;
}

int main(void) {
    /* Test case 1: Small buffer, few iterations */
    if (allocateAndFreeBuffer(1024, 5) == 0) {
        printf("Test 1 passed: 1KB buffer, 5 iterations\\n");
    }
    
    /* Test case 2: Medium buffer, moderate iterations */
    if (allocateAndFreeBuffer(8192, 10) == 0) {
        printf("Test 2 passed: 8KB buffer, 10 iterations\\n");
    }
    
    /* Test case 3: Larger buffer, single iteration */
    if (allocateAndFreeBuffer(65536, 1) == 0) {
        printf("Test 3 passed: 64KB buffer, 1 iteration\\n");
    }
    
    /* Test case 4: Small buffer, many iterations */
    if (allocateAndFreeBuffer(512, 100) == 0) {
        printf("Test 4 passed: 512B buffer, 100 iterations\\n");
    }
    
    /* Test case 5: Edge case - minimum valid size */
    if (allocateAndFreeBuffer(1, 3) == 0) {
        printf("Test 5 passed: 1B buffer, 3 iterations\\n");
    }
    
    return 0;
}
