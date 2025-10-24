
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Securely handles user input into a fixed-size buffer */
/* Returns true on success, false on failure */
bool handleUserInput(char* buffer, size_t bufferSize) {
    /* Validate input parameters - treat all inputs as untrusted */
    if (buffer == NULL || bufferSize == 0) {
        fprintf(stderr, "Error: Invalid buffer parameters\\n");
        return false;
    }
    
    /* Define a reasonable maximum input size to prevent excessive memory usage */
    const size_t MAX_INPUT_SIZE = 4096;
    if (bufferSize > MAX_INPUT_SIZE) {
        fprintf(stderr, "Error: Buffer size exceeds maximum allowed\\n");
        return false;
    }
    
    /* Initialize buffer to zero - prevent information leakage */
    memset(buffer, 0, bufferSize);
    
    /* Read input safely using fgets with explicit buffer size */
    /* fgets prevents buffer overflow by enforcing boundary checks */
    if (fgets(buffer, (int)bufferSize, stdin) == NULL) {
        if (feof(stdin)) {
            fprintf(stderr, "Error: End of file reached\\n");
        } else {
            fprintf(stderr, "Error: Failed to read input\\n");
        }
        return false;
    }
    
    /* Calculate actual length and check for truncation */
    size_t len = strlen(buffer);
    
    /* Remove trailing newline if present */
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    } else if (len == bufferSize - 1) {
        /* Input was likely truncated - clear remaining input */
        fprintf(stderr, "Warning: Input truncated to fit buffer size\\n");
        int c;
        while ((c = getchar()) != '\\n' && c != EOF);
    }
    
    /* Ensure null termination - defensive programming */
    buffer[bufferSize - 1] = '\\0';
    
    /* Validate that input contains only printable ASCII characters */
    /* This prevents injection of control characters */
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)buffer[i];
        if (c < 32 || c > 126) {
            fprintf(stderr, "Error: Input contains invalid characters\\n");
            /* Clear buffer on validation failure */
            memset(buffer, 0, bufferSize);
            return false;
        }
    }
    
    return true;
}

int main(void) {
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    
    printf("=== User Input Buffer Handler Test Cases ===\\n");
    
    /* Test case 1: Normal input */
    printf("\\nTest 1 - Enter normal text (e.g., 'Hello World'): ");
    fflush(stdout);
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        printf("Success: Input stored: '%s'\\n", buffer);
    } else {
        printf("Failed to handle input\\n");
    }
    
    /* Test case 2: Empty input */
    printf("\\nTest 2 - Press Enter for empty input: ");
    fflush(stdout);
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        printf("Success: Empty input handled, length: %zu\\n", strlen(buffer));
    } else {
        printf("Failed to handle input\\n");
    }
    
    /* Test case 3: Maximum length input */
    printf("\\nTest 3 - Enter text close to buffer size (try 250+ chars): ");
    fflush(stdout);
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        printf("Success: Input length: %zu\\n", strlen(buffer));
    } else {
        printf("Failed to handle input\\n");
    }
    
    /* Test case 4: Input with numbers and symbols */
    printf("\\nTest 4 - Enter text with numbers/symbols (e.g., 'Test123!@#'): ");
    fflush(stdout);
    if (handleUserInput(buffer, BUFFER_SIZE)) {
        printf("Success: Input stored: '%s'\\n", buffer);
    } else {
        printf("Failed to handle input\\n");
    }
    
    /* Test case 5: Small buffer size */
    printf("\\nTest 5 - Enter short text (using 32-byte buffer): ");
    fflush(stdout);
    char smallBuffer[32];
    /* Initialize small buffer to prevent information leakage */
    memset(smallBuffer, 0, sizeof(smallBuffer));
    if (handleUserInput(smallBuffer, sizeof(smallBuffer))) {
        printf("Success: Input stored: '%s'\\n", smallBuffer);
    } else {
        printf("Failed to handle input\\n");
    }
    
    return 0;
}
