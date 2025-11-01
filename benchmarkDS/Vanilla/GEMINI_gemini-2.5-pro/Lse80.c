#include <stdio.h>
#include <string.h>

/**
 * Reads a line from standard input into the provided buffer.
 * It reads at most (size - 1) characters. The newline is consumed
 * from the input stream but not stored in the buffer.
 *
 * @param buffer A character array to store the read string.
 * @param size The total size of the character array.
 * @return The number of characters read and stored in the buffer, or -1 on EOF.
 */
int scan(char* buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        // Handle EOF or read error
        buffer[0] = '\0';
        return -1;
    }

    size_t len = strlen(buffer);

    // Check if the last character is a newline
    if (len > 0 && buffer[len - 1] == '\n') {
        // It is, so remove it by replacing it with a null terminator
        buffer[len - 1] = '\0';
        return len - 1;
    } else {
        // The line was too long for the buffer, and fgets stopped reading.
        // The rest of the line is still in the input stream. We need to clear it.
        if (len == size - 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        return len;
    }
}

int main() {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    printf("Running 5 test cases for C scan function.\n");

    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        printf("Please enter a string: ");
        
        int charsRead = scan(buffer, BUFFER_SIZE);
        
        if (charsRead == -1) {
            printf("End of input reached.\n");
            break;
        }

        printf("Function returned (characters read): %d\n", charsRead);
        printf("Buffer content: \"%s\"\n", buffer);
        printf("strlen(buffer): %zu\n", strlen(buffer));
    }

    return 0;
}