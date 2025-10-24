#include <stdio.h>
#include <string.h>

// Define a fixed-size buffer for processing.
#define BUFFER_SIZE 64

/**
 * @brief Processes an input string by safely copying it to a fixed-size local buffer.
 *
 * This function demonstrates how to prevent buffer overflows in C when dealing
 * with fixed-size buffers. It uses strncpy to limit the number of bytes copied
 * and explicitly null-terminates the buffer to ensure it's a valid string.
 * If the source string is too long, it will be safely truncated.
 *
 * @param input A constant pointer to the null-terminated source string.
 */
void processString(const char* input) {
    char buffer[BUFFER_SIZE];

    printf("Original Input: \"%s\"\n", input);
    printf("Original Length: %zu\n", strlen(input));

    // Use strncpy to copy up to BUFFER_SIZE - 1 characters. This prevents
    // writing past the end of the buffer.
    strncpy(buffer, input, BUFFER_SIZE - 1);

    // CRITICAL STEP: strncpy does not guarantee null-termination if the source
    // string is as long as or longer than the size limit. We must manually
    // ensure the buffer is null-terminated.
    buffer[BUFFER_SIZE - 1] = '\0';

    printf("Content of safe buffer (truncated if needed): \"%s\"\n", buffer);
    printf("Length in buffer: %zu\n", strlen(buffer));
    printf("Processing complete.\n");
    printf("----------------------------------------\n");
}

int main() {
    printf("--- C Test Cases (Demonstrating Safe Handling) ---\n");

    // Test Case 1: String that fits comfortably in the buffer
    processString("This string fits perfectly.");

    // Test Case 2: An empty string
    processString("");

    // Test Case 3: A string that is exactly the size of the buffer (less one for null)
    processString("123456789012345678901234567890123456789012345678901234567890123");

    // Test Case 4: A very long string that will be truncated
    processString("This is a very long string that is intentionally designed to be much longer than the sixty-four byte buffer to demonstrate safe truncation.");

    // Test Case 5: A short string with special characters
    processString("Short & sweet!");

    return 0;
}