#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates a buffer of 1024 bytes on the heap.
 * It is the caller's responsibility to free this memory.
 * Exits on allocation failure for security.
 * @return A pointer to the allocated memory, or terminates if malloc fails.
 */
char* allocateBuffer() {
    char* buffer = (char*)malloc(1024);
    if (buffer == NULL) {
        perror("Fatal: Failed to allocate memory");
        exit(EXIT_FAILURE); // Terminate on memory allocation failure
    }
    return buffer;
}

/**
 * @brief Reads 10 characters from standard input and stores them in the buffer.
 * This function is secure against buffer overflows because it uses fread with
 * a fixed size, which will not read more than the specified number of bytes.
 * @param buf The buffer to store the read characters into. Must be non-NULL and
 *            have a capacity of at least 10 bytes.
 */
void readIntoBuffer(char* buf) {
    if (buf == NULL) {
        fprintf(stderr, "Error: Buffer is NULL.\n");
        return;
    }

    // fread is used for a bounded read. It reads 10 items of 1 byte each.
    size_t items_read = fread(buf, sizeof(char), 10, stdin);

    if (items_read < 10) {
        if (feof(stdin)) {
            fprintf(stderr, "Warning: End of input reached. Read only %zu bytes.\n", items_read);
        } else if (ferror(stdin)) {
            perror("Error reading from stdin");
        }
    }
    
    // Clear the rest of the input buffer until a newline or EOF is found.
    // This prevents leftover characters from being read in the next test case.
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        printf("Please enter at least 10 characters and press Enter: ");
        fflush(stdout);

        char* buffer = allocateBuffer();
        readIntoBuffer(buffer);

        // To display the read content, we print the first 10 characters.
        // We add a null terminator at position 10 to safely print as a string.
        char display_str[11];
        strncpy(display_str, buffer, 10);
        display_str[10] = '\0'; // Ensure null termination
        printf("First 10 bytes read into buffer: [%s]\n", display_str);

        // Securely free the allocated memory.
        free(buffer);
        buffer = NULL; // Prevent dangling pointer
    }
    printf("\nAll test cases finished.\n");
    return 0;
}