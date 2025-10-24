
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

// Convert string to integer with validation
// Returns 1 on success, 0 on failure
int stringToInt(const char* input, int* result) {
    // Rule #1: Initialize all pointers and validate parameters
    if (input == NULL || result == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to stringToInt\\n");
        return 0;
    }
    
    // Rule #3: Validate input is not empty
    size_t len = strlen(input);
    if (len == 0) {
        fprintf(stderr, "Error: Empty input string\\n");
        return 0;
    }
    
    // Rule #3: Check for reasonable length to prevent resource exhaustion
    if (len > 20) {
        fprintf(stderr, "Error: Input string too long\\n");
        return 0;
    }
    
    // Rule #3: Validate input contains only valid integer characters
    size_t start = 0;
    if (input[0] == '-' || input[0] == '+') {
        start = 1;
    }
    
    if (start >= len) {
        fprintf(stderr, "Error: Invalid input format\\n");
        return 0;
    }
    
    // Rule #4: Bounds checking on array access
    for (size_t i = start; i < len; ++i) {
        if (!isdigit((unsigned char)input[i])) {
            fprintf(stderr, "Error: Invalid character in input\\n");
            return 0;
        }
    }
    
    // Use strtol for safe conversion with error checking
    errno = 0;
    char* endptr = NULL;
    long value = strtol(input, &endptr, 10);
    
    // Rule #3: Check for conversion errors and Rule #1: Check return values
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
        fprintf(stderr, "Error: Number out of range for int\\n");
        return 0;
    }
    
    // Ensure entire string was consumed
    if (endptr != input + len) {
        fprintf(stderr, "Error: Invalid trailing characters\\n");
        return 0;
    }
    
    *result = (int)value;
    return 1;
}

int main(int argc, char* argv[]) {
    // Rule #1: Initialize all pointers
    char* testCases[] = {
        "42",
        "-123",
        "2147483647",
        "-2147483648",
        "invalid123"
    };
    
    printf("=== Test Cases ===\\n");
    for (int i = 0; i < 5; ++i) {
        int result = 0;
        printf("Input: \\"%s\\" -> ", testCases[i]);
        if (stringToInt(testCases[i], &result)) {
            printf("Success: %d\\n", result);
        } else {
            printf("Failed\\n");
        }
    }
    
    // Rule #1: Validate command line arguments
    if (argc < 2) {
        fprintf(stderr, "\\n=== Command Line Argument ===\\n");
        fprintf(stderr, "Usage: %s <integer_string>\\n", argv[0]);
        return 1;
    }
    
    printf("\\n=== Command Line Conversion ===\\n");
    int result = 0;
    
    if (stringToInt(argv[1], &result)) {
        printf("Successfully converted \\"%s\\" to: %d\\n", argv[1], result);
        return 0;
    } else {
        fprintf(stderr, "Failed to convert \\"%s\\"\\n", argv[1]);
        return 1;
    }
}
