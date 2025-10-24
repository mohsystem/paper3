
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/**
 * Safely converts a string to an integer with proper error handling
 * @param input The string to convert
 * @param result Pointer to store the converted integer
 * @return 1 if conversion successful, 0 otherwise
 */
int convertToInteger(const char* input, int* result) {
    // Input validation
    if (input == NULL || result == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return 0;
    }
    
    // Skip leading whitespace
    while (isspace(*input)) {
        input++;
    }
    
    if (*input == '\\0') {
        fprintf(stderr, "Error: Input is empty or contains only whitespace\\n");
        return 0;
    }
    
    // Validate format
    const char* ptr = input;
    if (*ptr == '+' || *ptr == '-') {
        ptr++;
    }
    
    if (*ptr == '\\0' || !isdigit(*ptr)) {
        fprintf(stderr, "Error: Invalid integer format: '%s'\\n", input);
        return 0;
    }
    
    // Use strtol for safe conversion
    char* endptr;
    errno = 0;
    long value = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
        fprintf(stderr, "Error: Value out of integer range: %s\\n", input);
        return 0;
    }
    
    // Check for trailing characters (skip trailing whitespace)
    while (isspace(*endptr)) {
        endptr++;
    }
    
    if (*endptr != '\\0') {
        fprintf(stderr, "Error: Invalid characters in input: '%s'\\n", input);
        return 0;
    }
    
    *result = (int)value;
    return 1;
}

int main(int argc, char* argv[]) {
    // Security: Validate command line arguments exist
    if (argc < 2) {
        fprintf(stderr, "Error: No command line arguments provided\\n");
        printf("Usage: %s <integer_string>\\n", argv[0]);
        return 1;
    }
    
    printf("=== Command Line Argument Conversion ===\\n");
    for (int i = 1; i < argc; i++) {
        printf("\\nArgument %d: %s\\n", i, argv[i]);
        int result;
        if (convertToInteger(argv[i], &result)) {
            printf("Converted successfully: %d\\n", result);
        }
    }
    
    // Test cases
    printf("\\n=== Running Test Cases ===\\n");
    const char* testCases[] = {
        "42",                    // Valid positive integer
        "-123",                  // Valid negative integer
        "0",                     // Zero
        "2147483647",           // MAX_VALUE
        "invalid123"            // Invalid format
    };
    
    for (int i = 0; i < 5; i++) {
        printf("\\nTest Case %d: \\"%s\\"\\n", i + 1, testCases[i]);
        int result;
        if (convertToInteger(testCases[i], &result)) {
            printf("Result: %d\\n", result);
        }
    }
    
    return 0;
}
