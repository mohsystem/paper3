#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define MAX_CHARS (BUFFER_SIZE - 1)

/**
 * Reads a line from standard input into the provided buffer.
 * It reads at most MAX_CHARS characters. If the line is longer,
 * the excess characters are discarded from the input stream.
 * The trailing newline character is removed.
 *
 * @param buffer A character array to store the read string.
 * @param size The total size of the buffer. Must be > 1.
 * @return The number of characters stored in the buffer (equivalent to strlen),
 *         or -1 on read error or EOF before any characters are read.
 */
int scan(char *buffer, int size) {
    if (buffer == NULL || size <= 1) {
        return -1;
    }
    
    // Read a line from stdin. fgets is safe as it respects the buffer size.
    // It reads up to size-1 characters and appends a null terminator.
    if (fgets(buffer, size, stdin) == NULL) {
        // This indicates an error or EOF.
        buffer[0] = '\0';
        return -1; 
    }

    // Find the newline character.
    char *newline_ptr = strchr(buffer, '\n');
    
    if (newline_ptr != NULL) {
        // Newline was found, so the entire line fit in the buffer.
        // Replace it with a null terminator.
        *newline_ptr = '\0';
    } else {
        // No newline was found. This means the line was too long and
        // was truncated. We must consume the rest of the line from stdin
        // to prevent it from being read by the next input operation.
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            // Discard characters
        }
    }
    
    return (int)strlen(buffer);
}

int main() {
    char buffer[BUFFER_SIZE];
    
    printf("--- Running 5 Test Cases ---\n");
    for (int i = 1; i <= 5; ++i) {
        printf("\nTest Case %d:\n", i);
        printf("Please enter a line of text (or press Ctrl+D/Ctrl+Z for EOF):\n");
        
        int chars_read = scan(buffer, BUFFER_SIZE);

        if (chars_read != -1) {
            printf("String read: \"%s\"\n", buffer);
            printf("Characters read: %d\n", chars_read);
        } else {
            printf("End of input reached or an error occurred.\n");
            break; // Exit loop on EOF or error
        }
    }
    printf("\n--- Tests Complete ---\n");
    
    return 0;
}