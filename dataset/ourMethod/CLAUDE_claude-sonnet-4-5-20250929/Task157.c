
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* Maximum allocation size to prevent excessive memory usage */
#define MAX_ALLOCATION_SIZE (1024UL * 1024UL * 100UL) /* 100 MB */

/* Secure memset that won't be optimized away - C11 memset_s alternative */
void secure_memzero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) {
        return;
    }
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Function to dynamically allocate and deallocate memory
 * Returns 1 on success, 0 on failure
 */
int allocateAndDeallocate(size_t size) {
    unsigned char *buffer = NULL;
    
    /* Input validation: check size is within safe bounds */
    if (size == 0) {
        fprintf(stderr, "Error: Allocation size must be greater than 0\\n");
        return 0;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        fprintf(stderr, "Error: Allocation size exceeds maximum allowed\\n");
        return 0;
    }
    
    /* Check for potential integer overflow in allocation */
    if (size > SIZE_MAX) {
        fprintf(stderr, "Error: Size exceeds SIZE_MAX\\n");
        return 0;
    }
    
    /* Allocate memory using calloc to initialize to zero */
    /* calloc is preferred as it initializes memory and checks for overflow */
    buffer = (unsigned char *)calloc(size, sizeof(unsigned char));
    
    /* Check if allocation succeeded */
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return 0;
    }
    
    /* Perform some operation to demonstrate usage with bounds checking */
    buffer[0] = 0xAA;
    if (size > 1) {
        /* Bounds check before accessing last element */
        buffer[size - 1] = 0xBB;
    }
    
    printf("Successfully allocated %zu bytes\\n", size);
    printf("First byte: 0x%02X\\n", buffer[0]);
    if (size > 1) {
        printf("Last byte: 0x%02X\\n", buffer[size - 1]);
    }
    
    /* Clear sensitive data before deallocation using secure function */
    /* This prevents data remanence attacks */
    secure_memzero(buffer, size);
    
    /* Deallocate memory */
    free(buffer);
    
    /* Set pointer to NULL after free to prevent use-after-free */
    buffer = NULL;
    
    printf("Memory deallocated successfully\\n");
    return 1;
}

int main(void) {
    printf("=== Memory Allocation Test Cases ===\\n");
    
    /* Test case 1: Small allocation */
    printf("\\nTest 1: Allocate 64 bytes\\n");
    if (!allocateAndDeallocate(64)) {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    /* Test case 2: Medium allocation */
    printf("\\nTest 2: Allocate 1024 bytes\\n");
    if (!allocateAndDeallocate(1024)) {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    /* Test case 3: Large allocation */
    printf("\\nTest 3: Allocate 1 MB\\n");
    if (!allocateAndDeallocate(1024UL * 1024UL)) {
        fprintf(stderr, "Test 3 failed\\n");
    }
    
    /* Test case 4: Invalid - zero size */
    printf("\\nTest 4: Allocate 0 bytes (should fail)\\n");
    if (!allocateAndDeallocate(0)) {
        printf("Test 4 correctly rejected invalid input\\n");
    }
    
    /* Test case 5: Invalid - exceeds maximum */
    printf("\\nTest 5: Allocate excessive size (should fail)\\n");
    if (!allocateAndDeallocate(MAX_ALLOCATION_SIZE + 1)) {
        printf("Test 5 correctly rejected excessive allocation\\n");
    }
    
    return 0;
}
