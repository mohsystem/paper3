#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocates two 1024-byte buffers and copies up to 100 characters
 * from the input strings into them.
 * @param input1 The first string to copy.
 * @param input2 The second string to copy.
 */
void processStrings(const char* input1, const char* input2) {
    // Allocate first 1024-byte buffer
    char* buf1 = (char*)malloc(1024 * sizeof(char));
    if (buf1 == NULL) {
        perror("Failed to allocate memory for buf1");
        return;
    }
    // Initialize buffer with null characters
    memset(buf1, 0, 1024);

    // Copy up to 100 characters from input1 into buf1.
    // strncpy will not null-terminate if strlen(input1) >= 100.
    // The memset above ensures the rest of the buffer is null, but we add
    // an explicit terminator for robustness.
    strncpy(buf1, input1, 100);
    if (strlen(input1) >= 100) {
        buf1[100] = '\0';
    }

    // Allocate second 1024-byte buffer
    char* buf2 = (char*)malloc(1024 * sizeof(char));
    if (buf2 == NULL) {
        perror("Failed to allocate memory for buf2");
        free(buf1); // Clean up already allocated memory
        return;
    }
    memset(buf2, 0, 1024);

    // Copy up to 100 characters from input2 into buf2
    strncpy(buf2, input2, 100);
     if (strlen(input2) >= 100) {
        buf2[100] = '\0';
    }

    printf("Buffer 1 content: \"%s\"\n", buf1);
    printf("Buffer 2 content: \"%s\"\n", buf2);

    // Clean up dynamically allocated memory
    free(buf1);
    free(buf2);
}

int main() {
    const char* testCases[][2] = {
        {"Hello World", "This is a test"},
        {"A short string", "A very very very very very very very very very very very very very very very very very very very long string that will be truncated."},
        {"", "An empty string for the first input."},
        {"Another test case with numbers 12345.", ""},
        {"Last test case! Using special characters: !@#$%^&*()", "And another one."}
    };

    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input 1: \"%s\"\n", testCases[i][0]);
        printf("Input 2: \"%s\"\n", testCases[i][1]);
        processStrings(testCases[i][0], testCases[i][1]);
        printf("---------------------\n\n");
    }

    return 0;
}