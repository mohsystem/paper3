#include <stdio.h>
#include <stdlib.h>

/**
 * Allocates a buffer of 1024 bytes on the heap.
 * It is the caller's responsibility to free this memory.
 * @return A pointer to the allocated memory, or NULL on failure.
 */
char* allocateBuffer() {
    // Allocate 1024 bytes. Using calloc also initializes memory to zero.
    char* buffer = (char*)malloc(1024 * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory");
    }
    return buffer;
}

/**
 * Reads 10 characters from standard input and stores them in the buffer.
 * fread is used for its safety, as it requires specifying the exact number
 * of items to read, preventing a buffer overflow.
 * @param buf The buffer to store the read characters.
 */
void readIntoBuffer(char* buf) {
    if (buf == NULL) {
        fprintf(stderr, "Error: Buffer is NULL.\n");
        return;
    }

    // Read 10 items of size 1 byte from stdin into the buffer.
    size_t itemsRead = fread(buf, sizeof(char), 10, stdin);

    if (itemsRead < 10) {
        if (feof(stdin)) {
            // Reached end of file before reading 10 characters.
            // This is not necessarily an error, but good to be aware of.
        } else if (ferror(stdin)) {
            perror("Error reading from stdin");
        }
    }

    // Clean the input buffer to prevent leftover characters from
    // affecting the next test case.
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        printf("--- Test Case %d ---\n", i);
        printf("Please enter exactly 10 characters and press Enter:\n");

        char* buffer = allocateBuffer();
        if (buffer == NULL) {
            // Error message already printed by allocateBuffer
            return 1; // Exit with an error code
        }

        readIntoBuffer(buffer);

        // Print the first 10 characters. Using "%.10s" is a safe way
        // with printf to print at most 10 characters from a non-null-terminated string.
        printf("First 10 bytes of buffer: [%.10s]\n\n", buffer);

        // Crucial security step: free the allocated memory to prevent memory leaks.
        free(buffer);
    }

    return 0;
}