#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_BUFFER_SIZE 65536 // 64 KB limit

/**
 * @brief Allocates a buffer, prints its contents, and frees it.
 * @param sizeStr The size of the buffer as a C-style string.
 */
void allocateAndPrintBuffer(const char* sizeStr) {
    if (sizeStr == NULL) {
        fprintf(stderr, "Error: Size argument cannot be null.\n");
        return;
    }

    char* end;
    errno = 0; // To distinguish success/failure after call
    long size_long = strtol(sizeStr, &end, 10);

    // Input validation
    if (end == sizeStr || *end != '\0') {
        fprintf(stderr, "Error: Invalid number format for size.\n");
        return;
    }
    if ((errno == ERANGE && (size_long == LONG_MAX || size_long == LONG_MIN)) || (errno != 0 && size_long == 0)) {
        fprintf(stderr, "Error: Size is out of range for a long.\n");
        return;
    }
    if (size_long < 0) {
        fprintf(stderr, "Error: Buffer size cannot be negative.\n");
        return;
    }
    if (size_long > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer size exceeds the maximum allowed limit of %d bytes.\n", MAX_BUFFER_SIZE);
        return;
    }
    
    size_t size = (size_t)size_long;
    unsigned char* buffer = NULL;

    // Use calloc to allocate and zero-initialize the buffer.
    if (size > 0) {
        buffer = (unsigned char*)calloc(size, sizeof(unsigned char));
        if (buffer == NULL) {
            fprintf(stderr, "Error: Failed to allocate buffer of size %zu.\n", size);
            return;
        }
    }

    printf("Allocated a buffer of size: %zu\n", size);
    printf("Buffer contents (hex):\n");
    if (size == 0) {
        printf("(empty)\n");
    } else {
        for (size_t i = 0; i < size; ++i) {
            printf("%02x ", buffer[i]);
            if ((i + 1) % 16 == 0 && i + 1 < size) {
                printf("\n");
            }
        }
        printf("\n");
    }
    
    printf("--------------------\n");

    // Free the allocated memory and prevent use-after-free
    free(buffer);
    buffer = NULL; 
}

/**
 * @brief Main entry point. Uses command-line argument or runs test cases.
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <buffer_size>\n", argv[0]);
        fprintf(stderr, "\nRunning test cases:\n");

        printf("Test Case 1: Valid size (16)\n");
        allocateAndPrintBuffer("16");

        printf("Test Case 2: Zero size\n");
        allocateAndPrintBuffer("0");
        
        printf("Test Case 3: Negative size (-5, invalid)\n");
        allocateAndPrintBuffer("-5");
        
        printf("Test Case 4: Non-numeric size ('abc', invalid)\n");
        allocateAndPrintBuffer("abc");
        
        printf("Test Case 5: Size too large (%d, invalid)\n", MAX_BUFFER_SIZE + 1);
        char large_size_str[20];
        snprintf(large_size_str, sizeof(large_size_str), "%d", MAX_BUFFER_SIZE + 1);
        allocateAndPrintBuffer(large_size_str);
        
        return 1;
    }

    allocateAndPrintBuffer(argv[1]);

    return 0;
}