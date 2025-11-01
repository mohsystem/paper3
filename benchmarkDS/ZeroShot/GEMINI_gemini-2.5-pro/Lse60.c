#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// A reasonable upper limit for buffer size to prevent Denial of Service attacks
#define MAX_BUFFER_SIZE (1024 * 1024) // 1MB

/**
 * @brief Allocates a buffer of a specified size and prints its contents.
 * @param size_long The desired size of the buffer.
 */
void processBuffer(long size_long) {
    printf("--- Processing request for buffer of size: %ld ---\n", size_long);

    if (size_long < 0) {
        fprintf(stderr, "Error: Buffer size cannot be negative.\n");
        printf("--- Finished processing ---\n\n");
        return;
    }
    
    if ((unsigned long)size_long > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Requested buffer size %ld exceeds maximum allowed size of %d.\n", size_long, MAX_BUFFER_SIZE);
        printf("--- Finished processing ---\n\n");
        return;
    }

    size_t size = (size_t)size_long;

    if (size == 0) {
        printf("Buffer of size 0 requested. No allocation performed.\n");
        printf("Buffer is empty.\n");
        printf("--- Finished processing ---\n\n");
        return;
    }

    // Use malloc to allocate uninitialized memory.
    // For secure, zero-initialized memory, use calloc(size, 1).
    char* buffer = (char*)malloc(size * sizeof(char));

    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate buffer of size %zu. Out of memory.\n", size);
        printf("--- Finished processing ---\n\n");
        return;
    }

    printf("Buffer of size %zu allocated successfully.\n", size);
    
    // WARNING: Printing uninitialized memory is a security risk (information disclosure).
    // The output will be unpredictable garbage data.
    printf("Buffer contents (uninitialized garbage): ");
    size_t print_limit = (size < 32) ? size : 32;
    for (size_t i = 0; i < print_limit; ++i) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    if (size > print_limit) {
        printf("...");
    }
    printf("\n");

    free(buffer); // Deallocate the memory
    printf("--- Finished processing ---\n\n");
}

/**
 * @brief Main entry point. Processes command-line arguments or runs test cases.
 * @param argc Argument count.
 * @param argv Argument vector. Expects a single integer for buffer size.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    if (argc == 2) {
        char* endptr;
        errno = 0;

        long size = strtol(argv[1], &endptr, 10);

        if (endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid number format. Please provide a valid integer.\n");
            return 1;
        }
        
        if (errno == ERANGE) {
            fprintf(stderr, "Error: Number is out of range for a long integer.\n");
            return 1;
        }

        processBuffer(size);

    } else if (argc == 1) {
        printf("No command-line argument provided. Running test cases.\n");
        // 5 test cases
        processBuffer(16);          // Test Case 1: A small, valid size
        processBuffer(0);           // Test Case 2: A zero size
        processBuffer(1);           // Test Case 3: A size of 1
        processBuffer(-10);         // Test Case 4: An invalid negative size
        processBuffer(2000000);     // Test Case 5: A size that exceeds our custom limit
    } else {
        fprintf(stderr, "Usage: %s <buffer_size>\n", argv[0]);
        return 1;
    }

    return 0;
}