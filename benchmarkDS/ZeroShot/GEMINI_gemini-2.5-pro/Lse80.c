#include <stdio.h>
#include <string.h>

/**
 * @brief Securely reads a line from stdin into a fixed-size buffer.
 *
 * This function uses fgets to prevent buffer overflows. It reads characters
 * until a newline, EOF, or until the buffer is full (size - 1 chars).
 * The newline character, if read, is removed from the buffer. If the input
 * line is longer than the buffer, the rest of the line is consumed and
 * discarded from the input stream.
 *
 * @param buffer A pointer to the character array where input is stored.
 * @param size The total size of the buffer (e.g., 1024).
 * @return The number of characters read and stored in the buffer. Returns -1 on error.
 */
int scan(char *buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return -1; // Indicate error for invalid arguments
    }
    buffer[0] = '\0'; // Ensure buffer is empty on entry

    // Use fgets for safe reading, it stops at size-1 characters or newline
    if (fgets(buffer, size, stdin) == NULL) {
        // This handles EOF or a read error
        return 0;
    }

    // Find the newline character, if it exists
    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        // Newline was found, so the entire line fit in the buffer.
        // Replace it with a null terminator as per the prompt's requirements.
        *newline = '\0';
    } else {
        // No newline was found. This means the input line was too long and
        // was truncated by fgets. We must clear the rest of the input buffer
        // to prevent it from being read by the next input operation.
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    return strlen(buffer);
}

int main() {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    const char* testPrompts[] = {
        "a short string (e.g., 'hello world'):",
        "an empty string (just press Enter):",
        "a string with exactly 1023 'a' characters:",
        "a string longer than 1023 'b' characters (e.g., 1030 'b's):",
        "a string with special characters and spaces (e.g., '  !@#$%^&*()  '):"
    };

    printf("Running 5 test cases. Please provide input for each.\n");

    for (int i = 0; i < 5; i++) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Please enter %s\n", testPrompts[i]);

        int charsRead = scan(buffer, BUFFER_SIZE);
        
        if (charsRead >= 0) {
            printf("Function returned (chars read): %d\n", charsRead);
            printf("Buffer content: \"%s\"\n", buffer);
            // %zu is the correct format specifier for size_t (from strlen)
            printf("Buffer strlen: %zu\n", strlen(buffer));
        } else {
            fprintf(stderr, "Test case failed due to an error.\n");
            break;
        }
    }

    return 0;
}