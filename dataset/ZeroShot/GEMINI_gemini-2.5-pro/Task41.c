#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 50

/**
 * Processes an input string by safely copying it into a fixed-size local buffer.
 * It uses snprintf to prevent buffer overflows by ensuring that no more bytes
 * are written than the buffer can hold.
 *
 * @param input A pointer to the null-terminated input string.
 */
void processString(const char* input) {
    char buffer[BUFFER_SIZE];
    
    printf("Processing input: \"%s\"\n", input);

    // Use snprintf to safely copy and format the string into the buffer.
    // It will not write more than BUFFER_SIZE bytes (including the null terminator),
    // thus preventing a buffer overflow.
    int written_chars = snprintf(buffer, BUFFER_SIZE, "Processed: %s", input);

    // It's good practice to check for truncation.
    if (written_chars >= BUFFER_SIZE) {
        printf("Notice: The input was too long and has been truncated.\n");
    }

    // Process the content in the safe buffer (e.g., convert to uppercase)
    for (int i = 0; buffer[i] != '\0'; i++) {
        buffer[i] = toupper((unsigned char)buffer[i]);
    }
    
    printf("Output: \"%s\"\n", buffer);
    printf("------------------------------------\n");
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: String that fits well within the buffer
    processString("Hello World");

    // Test Case 2: An empty string
    processString("");

    // Test Case 3: String with special characters
    processString("Test #123!");

    // Test Case 4: A string that is longer than the buffer to demonstrate safe truncation
    processString("This is a very long string designed to be longer than the destination buffer to show how snprintf prevents an overflow.");
    
    // Test Case 5: A string that nearly fills the buffer
    processString("This string is almost exactly the right size.");

    return 0;
}