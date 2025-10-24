
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OUTPUT 1024

char* executeCommand(const char* userInput) {
    static char output[MAX_OUTPUT];
    char command[256];
    FILE* pipe;
    
    // Clear output buffer
    memset(output, 0, MAX_OUTPUT);
    
    // Vulnerable: Direct use of user input in OS command
    snprintf(command, sizeof(command), "echo %s", userInput);
    
    #ifdef _WIN32
        pipe = _popen(command, "r");
    #else
        pipe = popen(command, "r");
    #endif
    
    if (pipe == NULL) {
        strcpy(output, "Error: Failed to execute command");
        return output;
    }
    
    // Read command output
    if (fgets(output, MAX_OUTPUT, pipe) != NULL) {
        // Remove trailing newline if present
        size_t len = strlen(output);
        if (len > 0 && output[len-1] == '\\n') {
            output[len-1] = '\\0';
        }
    }
    
    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif
    
    return output;
}

int main() {
    printf("=== OS Command Injection Demo ===\\n\\n");
    
    // Test case 1: Normal input
    printf("Test 1 - Normal input:\\n");
    printf("Input: 'Hello World'\\n");
    printf("Output: %s\\n\\n", executeCommand("Hello World"));
    
    // Test case 2: Simple text
    printf("Test 2 - Simple text:\\n");
    printf("Input: 'Testing123'\\n");
    printf("Output: %s\\n\\n", executeCommand("Testing123"));
    
    // Test case 3: Number input
    printf("Test 3 - Number input:\\n");
    printf("Input: '42'\\n");
    printf("Output: %s\\n\\n", executeCommand("42"));
    
    // Test case 4: Special characters
    printf("Test 4 - Special characters:\\n");
    printf("Input: 'test@example.com'\\n");
    printf("Output: %s\\n\\n", executeCommand("test@example.com"));
    
    // Test case 5: Multiple words
    printf("Test 5 - Multiple words:\\n");
    printf("Input: 'This is a test'\\n");
    printf("Output: %s\\n\\n", executeCommand("This is a test"));
    
    return 0;
}
