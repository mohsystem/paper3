
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

int convertToInteger(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        return 0;
    }
    
    // Trim leading whitespace
    while (isspace(*input)) {
        input++;
    }
    
    if (*input == '\\0') {
        return 0;
    }
    
    char* endptr;
    errno = 0;
    long val = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return 0;
    }
    
    // Check if any conversion was done and no extra characters
    if (endptr == input) {
        return 0;
    }
    
    // Skip trailing whitespace
    while (isspace(*endptr)) {
        endptr++;
    }
    
    // Check if entire string was converted
    if (*endptr != '\\0') {
        return 0;
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
        int result;
        if (convertToInteger(testCases[i], &result)) {
            printf("Input: '%s' -> Output: %d\\n", testCases[i], result);
        } else {
            printf("Input: '%s' -> Output: Invalid (cannot convert to integer)\\n", testCases[i]);
        }
    }
    
    // Interactive console input
    printf("\\nEnter a value to convert to integer:\\n");
    char userInput[256];
    if (fgets(userInput, sizeof(userInput), stdin) != NULL) {
        // Remove newline if present
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len-1] == '\\n') {
            userInput[len-1] = '\\0';
        }
        
        int result;
        if (convertToInteger(userInput, &result)) {
            printf("Successfully converted to integer: %d\\n", result);
        } else {
            printf("Error: Cannot convert '%s' to integer\\n", userInput);
        }
    }
    
    return 0;
}
