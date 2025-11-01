#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

/**
 * Reads a string from standard input using a secure scanf format string.
 * Reads up to (size - 1) characters or until a newline.
 * The rest of the input line, including the newline, is consumed and discarded.
 *
 * @param buffer The character buffer to store the result.
 * @param size The total size of the buffer.
 * @return The number of characters read into the buffer.
 */
int scan(char* buffer, int size) {
    if (size <= 1) { // Need space for at least one char and null terminator
        if (size > 0) buffer[0] = '\0';
        // Clear input buffer in case of invalid size to prevent issues
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 0;
    }

    char format[32];
    // Create format string like "%1023[^\n]" to prevent buffer overflow.
    snprintf(format, sizeof(format), "%%%d[^\n]", size - 1);

    int items_read = scanf(format, buffer);

    // After scanf, the input stream might still contain the rest of the
    // line (if it was too long) and the newline character.
    // We must consume all of it to ensure the next read starts fresh.
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (items_read == 1) {
        // scanf was successful and read something into the buffer.
        return strlen(buffer);
    } else {
        // scanf failed to read. This happens if the line was empty (just a newline)
        // or on EOF. Ensure the buffer is a valid empty string.
        buffer[0] = '\0';
        return 0;
    }
}

int main() {
    printf("Running 5 test cases. Please provide input for each.\n");
    char buffer[BUFFER_SIZE];

    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        printf("Enter a string: ");
        
        int chars_read = scan(buffer, BUFFER_SIZE);
        
        printf("String read: \"%s\"\n", buffer);
        printf("Number of characters read: %d\n", chars_read);
    }

    return 0;
}