
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* Maximum buffer size to prevent excessive memory allocation */
#define MAX_BUFFER_SIZE 10485760 /* 10 MB */

/**
 * Secure memset that cannot be optimized away by compiler
 */
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/**
 * Allocates and prints buffer contents
 * @param size The size of buffer to allocate
 * @return 0 on success, -1 on error
 */
int allocate_and_print_buffer(size_t size) {
    unsigned char* buffer = NULL;
    
    /* Enforce maximum size to prevent excessive memory usage */
    if (size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size exceeds maximum allowed\\n");
        return -1;
    }
    
    /* Allocate buffer with calloc (initializes to zero and checks overflow) */
    buffer = (unsigned char*)calloc(size, 1);
    
    /* Check allocation result */
    if (buffer == NULL && size > 0) {
        fprintf(stderr, "Error: Unable to allocate buffer\\n");
        return -1;
    }
    
    /* Print buffer contents (binary safe) */
    if (size > 0) {
        if (fwrite(buffer, 1, size, stdout) != size) {
            fprintf(stderr, "Error: Write operation failed\\n");
            secure_zero(buffer, size);
            free(buffer);
            return -1;
        }
        fflush(stdout);
    }
    
    /* Clear sensitive data before freeing */
    secure_zero(buffer, size);
    
    /* Free allocated memory */
    free(buffer);
    buffer = NULL;
    
    return 0;
}

int main(int argc, char* argv[]) {
    long long value;
    char* endptr = NULL;
    
    /* Test cases when no arguments provided */
    if (argc == 1) {
        printf("Running test cases:\\n");
        
        /* Test 1: Normal small buffer */
        printf("\\nTest 1: Size 10\\n");
        allocate_and_print_buffer(10);
        
        /* Test 2: Zero size buffer */
        printf("\\nTest 2: Size 0\\n");
        allocate_and_print_buffer(0);
        
        /* Test 3: Moderate size buffer */
        printf("\\nTest 3: Size 1024\\n");
        allocate_and_print_buffer(1024);
        
        /* Test 4: Size at boundary */
        printf("\\nTest 4: Size 4096\\n");
        allocate_and_print_buffer(4096);
        
        /* Test 5: Excessive size (should fail) */
        printf("\\nTest 5: Size exceeding maximum\\n");
        allocate_and_print_buffer(MAX_BUFFER_SIZE + 1);
        
        return 0;
    }
    
    /* Validate argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <buffer_size>\\n", argv[0]);
        return 1;
    }
    
    /* Parse and validate input */
    errno = 0;
    value = strtoll(argv[1], &endptr, 10);
    
    /* Check for conversion errors */
    if (endptr == argv[1] || *endptr != '\\0' || errno == ERANGE) {
        fprintf(stderr, "Error: Invalid buffer size\\n");
        return 1;
    }
    
    /* Check for negative or overflow values */
    if (value < 0 || value > (long long)MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size out of valid range\\n");
        return 1;
    }
    
    return (allocate_and_print_buffer((size_t)value) == 0) ? 0 : 1;
}
