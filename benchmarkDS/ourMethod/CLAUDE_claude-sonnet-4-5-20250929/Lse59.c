
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

// Maximum allowed buffer size (10MB)
#define MAX_BUFFER_SIZE (10 * 1024 * 1024)

// Allocates a buffer and fills it with 'A'
// Parameters:
//   buffer_size: the size of the buffer to allocate
//   out_buffer: pointer to receive the allocated buffer (caller must free)
// Returns: 0 on success, -1 on failure
int fillBufferWithA(int buffer_size, char** out_buffer) {
    // Initialize output pointer to NULL for safety
    if (out_buffer == NULL) {
        return -1;
    }
    *out_buffer = NULL;
    
    // Validate input: buffer size must be positive and within reasonable limits
    // to prevent excessive memory usage
    if (buffer_size <= 0 || buffer_size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size must be positive and not exceed 10MB\\n");
        return -1;
    }
    
    // Check for integer overflow before allocation
    if (buffer_size > SIZE_MAX / sizeof(char)) {
        fprintf(stderr, "Error: Buffer size would cause integer overflow\\n");
        return -1;
    }
    
    // Allocate buffer - using malloc for demonstration
    // Note: true stack allocation would use VLA or alloca, but those are unsafe
    // for variable sizes, so we use heap allocation with validation
    char* buffer = (char*)malloc(buffer_size * sizeof(char));
    
    // Check allocation result - must validate malloc return
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return -1;
    }
    
    // Fill buffer with 'A' - using memset for bounds-safe operation
    // memset is safe here because we control the size parameter and validated it
    memset(buffer, 'A', buffer_size);
    
    // Set output parameter
    *out_buffer = buffer;
    
    return 0;
}

int main(void) {
    char* result = NULL;
    
    // Test case 1: Small buffer
    printf("Test 1 - Size 10: ");
    if (fillBufferWithA(10, &result) == 0 && result != NULL) {
        fwrite(result, 1, 10, stdout);
        printf("\\n");
        free(result); // Free exactly once
        result = NULL;
    } else {
        printf("Failed\\n");
    }
    
    // Test case 2: Medium buffer
    printf("Test 2 - Size 100: ");
    if (fillBufferWithA(100, &result) == 0 && result != NULL) {
        printf("First 10 chars: ");
        fwrite(result, 1, 10, stdout);
        printf("\\n");
        free(result); // Free exactly once
        result = NULL;
    } else {
        printf("Failed\\n");
    }
    
    // Test case 3: Large buffer
    printf("Test 3 - Size 1000: ");
    if (fillBufferWithA(1000, &result) == 0 && result != NULL) {
        printf("Buffer allocated successfully\\n");
        free(result); // Free exactly once
        result = NULL;
    } else {
        printf("Failed\\n");
    }
    
    // Test case 4: Edge case - size 1
    printf("Test 4 - Size 1: ");
    if (fillBufferWithA(1, &result) == 0 && result != NULL) {
        fwrite(result, 1, 1, stdout);
        printf("\\n");
        free(result); // Free exactly once
        result = NULL;
    } else {
        printf("Failed\\n");
    }
    
    // Test case 5: Invalid input - negative size
    printf("Test 5 - Negative size: ");
    if (fillBufferWithA(-1, &result) == 0) {
        printf("Should have failed\\n");
        if (result != NULL) {
            free(result);
            result = NULL;
        }
    } else {
        printf("Correctly rejected negative size\\n");
    }
    
    return 0;
}
