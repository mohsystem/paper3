#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define READ_SIZE 10

/**
 * Allocates a buffer of BUFFER_SIZE bytes.
 * @return A pointer to the allocated buffer, or NULL on failure.
 */
char* allocateBuffer() {
    // Allocate memory and check for failure.
    char* buf = (char*)malloc(BUFFER_SIZE);
    if (buf == NULL) {
        perror("Failed to allocate memory");
    }
    return buf;
}

/**
 * Reads up to READ_SIZE characters from stdin into the buffer.
 * @param buf The buffer to read into.
 * @param buf_size The total size of the buffer.
 * @return The number of characters actually read.
 */
size_t readIntoBuffer(char* buf, size_t buf_size) {
    if (buf == NULL) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return 0;
    }
    // Ensure buffer is large enough for the read operation.
    if (buf_size < READ_SIZE) {
        fprintf(stderr, "Buffer is too small to read %d characters.\n", READ_SIZE);
        return 0;
    }

    printf("Please enter characters. The first %d will be read: ", READ_SIZE);
    fflush(stdout);

    // fread is a bounded read and is suitable for reading a fixed number of bytes.
    size_t items_read = fread(buf, sizeof(char), READ_SIZE, stdin);
    
    if (ferror(stdin)) {
        perror("Error reading from stdin");
        clearerr(stdin); // Clear the error indicator
        return items_read > 0 ? items_read : 0;
    }

    // Clear the rest of the input line to prepare for the next test case.
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Best practice: Null-terminate the data if it's to be treated as a string.
    // Ensure there is space for the null terminator.
    if (items_read < buf_size) {
        buf[items_read] = '\0';
    } else {
        // If buffer is exactly READ_SIZE, we can't null-terminate without overwriting.
        // In our case, buf_size is 1024, so this is safe.
        buf[READ_SIZE] = '\0';
    }

    return items_read;
}

/**
 * Main function with 5 test cases.
 */
int main() {
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        char* buffer = allocateBuffer();
        if (buffer == NULL) {
            return 1; // Exit if allocation failed.
        }

        size_t bytes_read = readIntoBuffer(buffer, BUFFER_SIZE);

        if (bytes_read > 0) {
            printf("Read %zu characters. Buffer content: \"", bytes_read);
            // Print characters one by one to handle non-null-terminated data safely.
            for(size_t j = 0; j < bytes_read; ++j) {
                putchar(buffer[j]);
            }
            printf("\"\n");
        } else if (feof(stdin)) {
            printf("End of file reached.\n");
            free(buffer);
            break;
        } else {
            printf("No characters were read.\n");
        }
        
        // Free the allocated memory to prevent leaks.
        free(buffer);
        buffer = NULL;
    }
    return 0;
}