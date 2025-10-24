
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

// Rule#1: Maximum input length to prevent buffer overflow
#define MAX_INPUT_LENGTH 1000
#define MAX_INT_STRING_LENGTH 15

// Function to safely convert string to integer
// Returns true if conversion succeeds, false otherwise
bool convertStringToInt(const char* input, int* result) {
    // Rule#1: Validate pointer is not NULL
    if (input == NULL || result == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to function\\n");
        return false;
    }
    
    // Rule#3: Validate input - check for empty string
    size_t len = strlen(input);
    if (len == 0) {
        fprintf(stderr, "Error: Empty input string\\n");
        return false;
    }
    
    // Rule#3: Validate input length to prevent overflow
    if (len > MAX_INT_STRING_LENGTH) {
        fprintf(stderr, "Error: Input string too long\\n");
        return false;
    }
    
    // Rule#3: Validate that string contains only valid characters
    size_t startPos = 0;
    if (input[0] == '-' || input[0] == '+') {
        startPos = 1;
        if (len == 1) {
            fprintf(stderr, "Error: Sign character without digits\\n");
            return false;
        }
    }
    
    // Rule#4: Bounds-checked validation loop
    for (size_t i = startPos; i < len; ++i) {
        if (!isdigit((unsigned char)input[i])) {
            fprintf(stderr, "Error: Invalid character in input\\n");
            return false;
        }
    }
    
    // Rule#2: Use safer conversion with error checking
    errno = 0;
    char* endPtr = NULL;
    long value = strtol(input, &endPtr, 10);
    
    // Rule#6: Check for conversion errors
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Integer overflow/underflow (ERANGE)\\n");
        return false;
    }
    
    // Rule#3: Check if value is within int range
    if (value < INT_MIN || value > INT_MAX) {
        fprintf(stderr, "Error: Value out of integer range\\n");
        return false;
    }
    
    // Rule#3: Check if entire string was consumed
    if (endPtr == NULL || *endPtr != '\\0') {
        fprintf(stderr, "Error: Partial conversion or invalid input\\n");
        return false;
    }
    
    *result = (int)value;
    return true;
}

int main(void) {
    // Test cases
    const char* testCases[] = {
        "12345",
        "-9876",
        "0",
        "2147483647",
        "invalid123"
    };
    
    printf("Running test cases:\\n");
    for (int i = 0; i < 5; ++i) {
        printf("\\nTest %d: Input = \\"%s\\"\\n", i + 1, testCases[i]);
        int result = 0;
        if (convertStringToInt(testCases[i], &result)) {
            printf("Success: Converted to %d\\n", result);
        } else {
            printf("Failed to convert\\n");
        }
    }
    
    // Rule#3: Interactive user input with validation
    printf("\\n--- Interactive Mode ---\\n");
    printf("Enter an integer: ");
    
    // Rule#1,#4: Use safe input method with bounds checking
    char userInput[MAX_INPUT_LENGTH + 1];
    // Rule#1: Initialize buffer to zero
    memset(userInput, 0, sizeof(userInput));
    
    // Rule#2,#4: Use fgets instead of gets (safer)
    if (fgets(userInput, sizeof(userInput), stdin) != NULL) {
        // Rule#4: Check for buffer overflow condition
        size_t len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\\n') {
            // Remove newline character
            userInput[len - 1] = '\\0';
            len--;
        } else if (len == MAX_INPUT_LENGTH) {
            // Rule#4: Input was too long and truncated
            fprintf(stderr, "Error: Input too long (max %d characters)\\n", MAX_INPUT_LENGTH);
            // Clear remaining input from buffer
            int c;
            while ((c = getchar()) != '\\n' && c != EOF);
            return 1;
        }
        
        // Rule#3: Validate input length
        if (len == 0) {
            fprintf(stderr, "Error: Empty input\\n");
            return 1;
        }
        
        int result = 0;
        if (convertStringToInt(userInput, &result)) {
            printf("Successfully converted to integer: %d\\n", result);
        } else {
            fprintf(stderr, "Conversion failed\\n");
            return 1;
        }
    } else {
        // Rule#6: Handle input error
        fprintf(stderr, "Error reading input\\n");
        return 1;
    }
    
    return 0;
}
