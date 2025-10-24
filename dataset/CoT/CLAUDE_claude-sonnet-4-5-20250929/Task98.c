
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

int convertToInteger(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        return 0;
    }
    
    // Trim leading whitespace
    while (isspace((unsigned char)*input)) {
        input++;
    }
    
    if (*input == '\\0') {
        return 0;
    }
    
    char* endptr;
    errno = 0;
    long temp = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || temp > INT_MAX || temp < INT_MIN) {
        return 0;
    }
    
    // Check if entire string was converted
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }
    
    if (*endptr != '\\0') {
        return 0;
    }
    
    *result = (int)temp;
    return 1;
}

int main() {
    const char* testCases[] = {"123", "-456", "0", "abc", "2147483648"};
    int numTests = 5;
    
    printf("Running test cases:\\n");
    for (int i = 0; i < numTests; i++) {
        int result;
        if (convertToInteger(testCases[i], &result)) {
            printf("Input: '%s' -> Output: %d\\n", testCases[i], result);
        } else {
            printf("Input: '%s' -> Invalid input\\n", testCases[i]);
        }
    }
    
    printf("\\n--- Interactive Mode ---\\n");
    printf("Enter a value to convert to integer: ");
    
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
            printf("Error: Invalid input. Please enter a valid integer.\\n");
        }
    }
    
    return 0;
}
