
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Portable explicit zero function for secure memory clearing
// Uses volatile to prevent compiler optimization from removing the operation
static void explicit_zero(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

void demonstrate_memory_copy(size_t size) {
    unsigned char* chunk1 = NULL;
    unsigned char* chunk2 = NULL;
    
    // Input validation: check for reasonable size and prevent overflow
    if (size == 0 || size > 1024 * 1024) { // Limit to 1MB to prevent excessive allocation
        fprintf(stderr, "Invalid size\\n");
        return;
    }
    
    // Allocate first chunk with NULL check
    chunk1 = (unsigned char*)malloc(size);
    if (chunk1 == NULL) {
        fprintf(stderr, "Memory allocation failed for chunk1\\n");
        return;
    }
    
    // Initialize chunk1 with sample data (bounds are guaranteed by allocation)
    for (size_t i = 0; i < size; i++) {
        chunk1[i] = (unsigned char)(i % 256);
    }
    
    // Allocate second chunk with NULL check
    chunk2 = (unsigned char*)malloc(size);
    if (chunk2 == NULL) {
        explicit_zero(chunk1, size); // Clear before free
        free(chunk1);
        fprintf(stderr, "Memory allocation failed for chunk2\\n");
        return;
    }
    
    // Securely copy contents from chunk1 to chunk2 with bounds checking
    // Both buffers are same size so memcpy is safe here
    memcpy(chunk2, chunk1, size);
    
    // Print contents of chunk2 (limited to first 10 bytes to prevent excessive output)
    printf("Contents of chunk2 after copy: ");
    for (size_t i = 0; i < size && i < 10; i++) {
        printf("%d ", chunk2[i]);
    }
    printf("\\n");
    
    // Clear chunk1 before freeing (critical for sensitive data)
    explicit_zero(chunk1, size);
    free(chunk1);
    chunk1 = NULL; // Prevent use after free
    
    // Print contents of chunk2 again (still valid and accessible)
    printf("Contents of chunk2 after chunk1 freed: ");
    for (size_t i = 0; i < size && i < 10; i++) {
        printf("%d ", chunk2[i]);
    }
    printf("\\n");
    
    // Clear and free chunk2
    explicit_zero(chunk2, size);
    free(chunk2);
    chunk2 = NULL; // Prevent use after free
}

int main(void) {
    // Test case 1: Small allocation
    printf("Test 1: Size 10\\n");
    demonstrate_memory_copy(10);
    
    // Test case 2: Medium allocation
    printf("\\nTest 2: Size 100\\n");
    demonstrate_memory_copy(100);
    
    // Test case 3: Larger allocation
    printf("\\nTest 3: Size 1000\\n");
    demonstrate_memory_copy(1000);
    
    // Test case 4: Edge case - size 1
    printf("\\nTest 4: Size 1\\n");
    demonstrate_memory_copy(1);
    
    // Test case 5: Invalid case - size 0
    printf("\\nTest 5: Size 0 (invalid)\\n");
    demonstrate_memory_copy(0);
    
    return 0;
}
