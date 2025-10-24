#include <stdio.h>
#include <string.h>

/**
 * @brief Safely copies a string into a fixed-size character buffer.
 * 
 * This function uses snprintf, which is a secure way to prevent buffer overflows.
 * It guarantees that the destination buffer will not be overflowed and will be
 * null-terminated.
 *
 * @param buffer The destination character array (the buffer).
 * @param bufferSize The total size of the destination buffer.
 * @param input The source string to copy from.
 */
void handleInput(char* buffer, size_t bufferSize, const char* input) {
    if (buffer == NULL || bufferSize == 0 || input == NULL) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return;
    }
    // snprintf is a safe C library function. It will write at most
    // bufferSize - 1 characters to the buffer and will always
    // null-terminate the result, preventing a buffer overflow.
    snprintf(buffer, bufferSize, "%s", input);
}

int main() {
    const size_t BUFFER_SIZE = 20;

    const char* testCases[] = {
        "",                                         // Empty string
        "short",                                    // Shorter than buffer
        "This is exactly 19!",                      // Exactly buffer size - 1
        "This input is too long for the buffer",    // Longer than buffer
        "This is a very very very very very very very long input string that will surely be truncated" // Very long string
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("C Test Cases (Buffer Size: %zu)\n", BUFFER_SIZE);
    printf("------------------------------------\n");

    for (int i = 0; i < numTestCases; ++i) {
        // Define a buffer on the stack for each test case.
        char buffer[BUFFER_SIZE];
        
        printf("Test Case %d:\n", i + 1);
        printf("Input: \"%s\"\n", testCases[i]);
        
        handleInput(buffer, BUFFER_SIZE, testCases[i]);
        
        printf("Buffer Content: \"%s\"\n", buffer);
        printf("\n");
    }

    return 0;
}