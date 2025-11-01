
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Reads a line from standard input with maximum length limit
// Returns number of characters read (excluding newline)
// Maximum 1023 characters to fit in 1024-byte buffer (with null terminator)
int readLimitedLine(char* buffer, int maxLen) {
    // Validate input parameters - bounds checking to prevent buffer overflow
    if (buffer == NULL || maxLen <= 0) {
        return -1; /* Fail closed on invalid input */
    }
    
    // Initialize buffer to zeros - secure memory initialization
    memset(buffer, 0, maxLen + 1);
    
    // Use fgets for safe input - prevents buffer overflow
    // fgets reads at most maxLen characters and adds null terminator
    // This is safer than scanf which doesn't enforce length limits properly\n    if (fgets(buffer, maxLen + 1, stdin) == NULL) {\n        return 0; /* No input available or error occurred */\n    }\n    \n    // Calculate actual characters read with explicit bounds check\n    size_t len = strlen(buffer);\n    \n    // Remove trailing newline - requirement states newline not stored\n    if (len > 0 && buffer[len - 1] == '\
') {\n        buffer[len - 1] = '\\0';\n        len--;\n    }\n    // Handle CRLF line endings\n    if (len > 0 && buffer[len - 1] == '\\r') {\n        buffer[len - 1] = '\\0';\n        len--;\n    }\n    \n    // Enforce maximum length constraint - additional safety validation\n    if (len > (size_t)maxLen) {\n        len = maxLen;\n        buffer[maxLen] = '\\0'; /* Ensure null termination */\n    }\n    \n    return (int)len;\n}\n\nint main(void) {\n    const int BUFFER_SIZE = 1024;\n    const int MAX_INPUT = 1023;\n    \n    printf("Test Case 1: Normal input (type 'Hello' and press Enter)\\n");\n    char buffer1[BUFFER_SIZE];\n    int result1 = readLimitedLine(buffer1, MAX_INPUT);\n    printf("Characters read: %d, Content: '%s'\\n\\n", result1, buffer1);\n    \n    printf("Test Case 2: Empty input (press Enter only)\\n");\n    char buffer2[BUFFER_SIZE];\n    int result2 = readLimitedLine(buffer2, MAX_INPUT);\n    printf("Characters read: %d, Content: '%s'\\n\\n", result2, buffer2);\n    \n    printf("Test Case 3: Special characters (type 'Test@123!#')\\n");\n    char buffer3[BUFFER_SIZE];\n    int result3 = readLimitedLine(buffer3, MAX_INPUT);\n    printf("Characters read: %d, Content: '%s'\\n\\n", result3, buffer3);\n    \n    printf("Test Case 4: Long input (type many characters)\\n");\n    char buffer4[BUFFER_SIZE];\n    int result4 = readLimitedLine(buffer4, MAX_INPUT);\n    printf("Characters read: %d (max %d)\\n\\n", result4, MAX_INPUT);\n    \n    printf("Test Case 5: Whitespace (type spaces)\\n");\n    char buffer5[BUFFER_SIZE];\n    int result5 = readLimitedLine(buffer5, MAX_INPUT);\n    printf("Characters read: %d, Content: '%s'\
", result5, buffer5);
    
    return 0;
}
