
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

/**
 * Safely handles user input into a fixed-size buffer
 * @param input The user input string
 * @param buffer The output buffer (must be BUFFER_SIZE or larger)
 * @return The length of the copied string
 */
int handleInputToBuffer(const char* input, char* buffer) {
    if (input == NULL || buffer == NULL) {
        if (buffer != NULL) {
            buffer[0] = '\\0';
        }
        return 0;
    }
    
    // Initialize buffer with null bytes
    memset(buffer, 0, BUFFER_SIZE);
    
    // Calculate safe copy length
    size_t inputLen = strlen(input);
    size_t copyLength = (inputLen < BUFFER_SIZE - 1) ? inputLen : (BUFFER_SIZE - 1);
    
    // Safely copy with bounds checking using strncpy
    strncpy(buffer, input, copyLength);
    
    // Ensure null termination
    buffer[copyLength] = '\\0';
    
    return (int)copyLength;
}

/**
 * Helper function to create a string of repeated characters
 */
char* createRepeatedString(char c, int length) {
    char* str = (char*)malloc(length + 1);
    if (str != NULL) {
        memset(str, c, length);
        str[length] = '\\0';
    }
    return str;
}

int main() {
    printf("=== Secure Buffer Input Handler - C ===\\n\\n");
    
    char buffer[BUFFER_SIZE];
    
    // Test case 1: Normal input
    const char* test1 = "Hello, World!";
    printf("Test 1 - Normal input:\\n");
    printf("Input: %s\\n", test1);
    handleInputToBuffer(test1, buffer);
    printf("Output: %s\\n\\n", buffer);
    
    // Test case 2: Empty input
    const char* test2 = "";
    printf("Test 2 - Empty input:\\n");
    printf("Input: (empty)\\n");
    handleInputToBuffer(test2, buffer);
    printf("Output: %s\\n\\n", buffer);
    
    // Test case 3: Input at buffer limit
    char* test3 = createRepeatedString('A', 99);
    printf("Test 3 - Input at buffer limit (99 chars):\\n");
    printf("Input length: %d\\n", 99);
    int len3 = handleInputToBuffer(test3, buffer);
    printf("Output length: %d\\n\\n", len3);
    free(test3);
    
    // Test case 4: Input exceeding buffer size (security test)
    char* test4 = createRepeatedString('B', 150);
    printf("Test 4 - Input exceeding buffer (150 chars):\\n");
    printf("Input length: %d\\n", 150);
    int len4 = handleInputToBuffer(test4, buffer);
    printf("Output length: %d\\n", len4);
    printf("Safely truncated: %s\\n\\n", (len4 < 150) ? "true" : "false");
    free(test4);
    
    // Test case 5: NULL input
    printf("Test 5 - NULL input:\\n");
    printf("Input: NULL\\n");
    handleInputToBuffer(NULL, buffer);
    printf("Output: %s\\n\\n", buffer);
    
    return 0;
}
