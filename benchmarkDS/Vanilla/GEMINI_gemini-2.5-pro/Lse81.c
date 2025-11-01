#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates a buffer of 1024 bytes on the heap.
 * @return A pointer to the allocated buffer, or NULL on failure.
 *         The caller is responsible for freeing this memory using free().
 */
char* allocateBuffer() {
    char* buffer = (char*)malloc(1024);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
    }
    return buffer;
}

/**
 * @brief Reads a line from stdin and stores the first 10 characters in the buffer.
 * @param buf A pointer to a buffer of at least 10 bytes capacity.
 */
void readIntoBuffer(char* buf) {
    if (buf == NULL) {
        fprintf(stderr, "Error: Buffer is NULL.\n");
        return;
    }
    
    char temp_input[256];
    if (fgets(temp_input, sizeof(temp_input), stdin) != NULL) {
        // Copy up to 10 characters into the destination buffer.
        // strncpy is used for safety to not write past the 10-byte boundary.
        strncpy(buf, temp_input, 10);
    }
}

int main() {
    printf("Running 5 test cases for C...\n");
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        
        // 1. Allocate buffer
        char* buffer = allocateBuffer();
        if (buffer == NULL) {
            return 1; // Exit if allocation fails
        }
        // Initialize buffer to a known state (zeros)
        memset(buffer, 0, 1024);
        
        // 2. Read into buffer
        printf("Enter some characters (up to 10 will be read) and press Enter: ");
        readIntoBuffer(buffer);
        
        // 3. Print the result
        printf("First 10 bytes of buffer (as chars): [");
        for (int j = 0; j < 10; ++j) {
            char c = buffer[j];
            // Print non-printable characters (like newline, null) as a dot
            if (c < 32) {
                putchar('.');
            } else {
                putchar(c);
            }
        }
        printf("]\n");
        
        printf("First 10 byte values (as ints):    [");
        for (int j = 0; j < 10; ++j) {
            printf("%d%s", (int)buffer[j], (j < 9 ? ", " : ""));
        }
        printf("]\n");

        // 4. Free allocated memory
        free(buffer);
    }
    return 0;
}