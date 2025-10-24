
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 256
#define MAX_OUTPUT_LENGTH 4096
#define MAX_COMMAND_LENGTH 512

// Security: Validate input to contain only safe characters
// This prevents command injection by rejecting shell metacharacters
bool isValidInput(const char* input) {
    if (input == NULL) {
        return false;
    }
    
    size_t len = strlen(input);
    if (len == 0 || len > MAX_INPUT_LENGTH) {
        return false; // Reject empty or excessively long inputs
    }
    
    // Only allow alphanumeric characters, dash, underscore, and dot
    // This whitelist approach prevents injection of shell metacharacters
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            return false;
        }
    }
    
    // Security: Prevent directory traversal attacks
    if (strstr(input, "..") != NULL) {
        return false;
    }
    
    return true;
}

// Security: Execute command with validated input
// Returns allocated string with result (caller must free) or NULL on error
char* executeCommand(const char* userInput) {
    // Security: Validate input before using in command
    if (!isValidInput(userInput)) {
        char* error = (char*)malloc(128);
        if (error == NULL) {
            return NULL;
        }
        snprintf(error, 128, "Error: Invalid input. Only alphanumeric, dash, underscore, dot allowed.");
        return error;
    }
    
    // Security: Construct command safely with bounds checking
    char command[MAX_COMMAND_LENGTH];
    int written = snprintf(command, MAX_COMMAND_LENGTH, "echo \\"Processing: %s\\"", userInput);
    
    // Security: Check for truncation
    if (written < 0 || written >= MAX_COMMAND_LENGTH) {
        char* error = (char*)malloc(64);
        if (error == NULL) {
            return NULL;
        }
        snprintf(error, 64, "Error: Command too long");
        return error;
    }
    
    // Security: Execute command in read-only mode
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        char* error = (char*)malloc(64);
        if (error == NULL) {
            return NULL;
        }
        snprintf(error, 64, "Error: Failed to execute command");
        return error;
    }
    
    // Security: Allocate output buffer with size limit
    char* result = (char*)calloc(MAX_OUTPUT_LENGTH + 1, 1);
    if (result == NULL) {
        pclose(pipe);
        return NULL;
    }
    
    size_t total_read = 0;
    char buffer[128];
    
    // Security: Read output with bounds checking
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && total_read < MAX_OUTPUT_LENGTH) {
        size_t buf_len = strlen(buffer);
        size_t remaining = MAX_OUTPUT_LENGTH - total_read;
        
        if (buf_len > remaining) {
            buf_len = remaining;
        }
        
        memcpy(result + total_read, buffer, buf_len);
        total_read += buf_len;
    }
    
    result[total_read] = '\\0'; // Ensure null termination
    pclose(pipe);
    
    return result;
}

int main(void) {
    printf("Secure Command Execution Demo\\n");
    printf("==============================\\n\\n");
    
    // Test case 1: Valid alphanumeric input
    const char* test1 = "testfile123";
    printf("Test 1 - Valid input: %s\\n", test1);
    char* result1 = executeCommand(test1);
    if (result1 != NULL) {
        printf("Result: %s\\n", result1);
        free(result1); // Security: Free allocated memory
        result1 = NULL;
    }
    printf("\\n");
    
    // Test case 2: Valid input with allowed characters
    const char* test2 = "my-file_name.txt";
    printf("Test 2 - Valid filename: %s\\n", test2);
    char* result2 = executeCommand(test2);
    if (result2 != NULL) {
        printf("Result: %s\\n", result2);
        free(result2);
        result2 = NULL;
    }
    printf("\\n");
    
    // Test case 3: Invalid input with shell metacharacter (injection attempt)
    const char* test3 = "file; rm -rf /";
    printf("Test 3 - Injection attempt: %s\\n", test3);
    char* result3 = executeCommand(test3);
    if (result3 != NULL) {
        printf("Result: %s\\n", result3);
        free(result3);
        result3 = NULL;
    }
    printf("\\n");
    
    // Test case 4: Invalid input with pipe character
    const char* test4 = "test|cat /etc/passwd";
    printf("Test 4 - Pipe injection attempt: %s\\n", test4);
    char* result4 = executeCommand(test4);
    if (result4 != NULL) {
        printf("Result: %s\\n", result4);
        free(result4);
        result4 = NULL;
    }
    printf("\\n");
    
    // Test case 5: Invalid input with directory traversal
    const char* test5 = "../../../etc/passwd";
    printf("Test 5 - Directory traversal attempt: %s\\n", test5);
    char* result5 = executeCommand(test5);
    if (result5 != NULL) {
        printf("Result: %s\\n", result5);
        free(result5);
        result5 = NULL;
    }
    printf("\\n");
    
    return 0;
}
