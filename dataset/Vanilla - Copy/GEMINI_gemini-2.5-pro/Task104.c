#include <stdio.h>
#include <string.h>

void handleInput(const char* input, char* buffer, size_t bufferSize) {
    if (input == NULL || buffer == NULL || bufferSize == 0) {
        if (buffer != NULL && bufferSize > 0) {
           buffer[0] = '\0';
        }
        return;
    }
    
    // strncpy copies at most n characters and may not null-terminate.
    strncpy(buffer, input, bufferSize - 1);
    
    // Explicitly null-terminate the end of the buffer to be safe.
    buffer[bufferSize - 1] = '\0';
}

int main() {
    const size_t BUFFER_SIZE = 16;
    const char* testCases[] = {
        "Hello World",
        "This is a long string that will be truncated",
        "123456789012345", // Exactly 15 characters to fit with '\0'
        "",
        "Another long test for truncation"
    };
    
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char buffer[BUFFER_SIZE];
        handleInput(testCases[i], buffer, BUFFER_SIZE);
        printf("Input:  \"%s\"\n", testCases[i]);
        printf("Output: \"%s\"\n\n", buffer);
    }

    return 0;
}