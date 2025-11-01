
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_BUFFER_SIZE (1024 * 1024) // 1MB limit

void allocateAndPrintBuffer(long long size) {
    // Input validation
    if (size <= 0) {
        printf("Error: Buffer size must be positive\\n");
        return;
    }
    
    if (size > MAX_BUFFER_SIZE) {
        printf("Error: Buffer size exceeds maximum allowed (%d bytes)\\n", MAX_BUFFER_SIZE);
        return;
    }
    
    // Allocate buffer using calloc (initializes to zero)
    unsigned char* buffer = (unsigned char*)calloc((size_t)size, sizeof(unsigned char));
    
    if (buffer == NULL) {
        printf("Error: Unable to allocate buffer - out of memory\\n");
        return;
    }
    
    // Print buffer contents (safely, limit output)
    printf("Buffer contents (%lld bytes): [", size);
    size_t display_size = (size < 100) ? (size_t)size : 100;
    
    for (size_t i = 0; i < display_size; i++) {
        printf("%d", buffer[i]);
        if (i < display_size - 1) printf(", ");
    }
    if (size > 100) printf("...");
    printf("]\\n");
    
    // Free allocated memory
    free(buffer);
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1 (valid size 10):\\n");
    allocateAndPrintBuffer(10);
    printf("\\n");
    
    printf("Test Case 2 (valid size 50):\\n");
    allocateAndPrintBuffer(50);
    printf("\\n");
    
    printf("Test Case 3 (negative size):\\n");
    allocateAndPrintBuffer(-5);
    printf("\\n");
    
    printf("Test Case 4 (zero size):\\n");
    allocateAndPrintBuffer(0);
    printf("\\n");
    
    printf("Test Case 5 (excessive size):\\n");
    allocateAndPrintBuffer(MAX_BUFFER_SIZE + 1);
    printf("\\n");
    
    // Command line argument handling
    if (argc > 1) {
        char* endptr;
        errno = 0;
        long long size = strtoll(argv[1], &endptr, 10);
        
        if (errno != 0 || *endptr != '\\0' || endptr == argv[1]) {
            printf("Error: Invalid number format\\n");
        } else {
            printf("Command line argument:\\n");
            allocateAndPrintBuffer(size);
        }
    }
    
    return 0;
}
