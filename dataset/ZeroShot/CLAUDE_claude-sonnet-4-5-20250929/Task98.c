
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

int convertToInteger(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        printf("Error: NULL input\\n");
        return 0;
    }
    
    // Trim leading whitespace
    while (isspace(*input)) {
        input++;
    }
    
    if (*input == '\\0') {
        printf("Error: Invalid input. Cannot convert empty string to integer.\\n");
        return 0;
    }
    
    char* endptr;
    errno = 0;
    long val = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        printf("Error: Number out of range. Cannot convert '%s' to integer.\\n", input);
        return 0;
    }
    
    // Check if any characters were converted
    if (endptr == input) {
        printf("Error: Invalid input. Cannot convert '%s' to integer.\\n", input);
        return 0;
    }
    
    // Check for trailing non-whitespace characters
    while (*endptr != '\\0') {
        if (!isspace(*endptr)) {
            printf("Error: Invalid input. Cannot convert '%s' to integer.\\n", input);
            return 0;
        }
        endptr++;
    }
    
    *result = (int)val;
    return 1;
}

int main() {
    // Test cases
    const char* testCases[] = {"123", "-456", "0", "abc", "12.34"};
    int numTests = 5;
    
    printf("Running test cases:\\n");
    for (int i = 0; i < numTests; i++) {
        printf("Input: '%s' -> ", testCases[i]);
        int result;
        if (convertToInteger(testCases[i], &result)) {
            printf("Output: %d\\n", result);
        }
    }
    
    // Interactive input
    printf("\\nEnter a value to convert to integer (or 'exit' to quit):\\n");
    char input[256];
    
    while (fgets(input, sizeof(input), stdin)) {
        // Remove newline character
        input[strcspn(input, "\\n")] = '\\0';
        
        if (strcmp(input, "exit") == 0) {
            break;
        }
        
        int result;
        if (convertToInteger(input, &result)) {
            printf("Successfully converted to: %d\\n", result);
        }
        printf("Enter another value (or 'exit' to quit):\\n");
    }
    
    return 0;
}
