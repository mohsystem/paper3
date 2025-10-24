
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define MAX_INPUT_LENGTH 100
#define MIN_INPUT_LENGTH 1
#define MAX_INT_DIGITS 10

// Validates that input string length is within acceptable bounds
bool validateStringLength(const char* input, size_t minLen, size_t maxLen) {
    if (input == NULL) return false;
    
    size_t len = strlen(input);
    return len >= minLen && len <= maxLen;
}

// Validates that string contains only alphanumeric characters and spaces
bool validateAlphanumeric(const char* input) {
    if (input == NULL || input[0] == '\\0') return false;
    
    for (size_t i = 0; input[i] != '\\0'; i++) {
        unsigned char c = (unsigned char)input[i];
        if (!isalnum(c) && c != ' ') {
            return false;
        }
    }
    return true;
}

// Validates integer input within specified range
bool validateInteger(int value, int minVal, int maxVal) {
    return value >= minVal && value <= maxVal;
}

// Safe integer input function with validation
bool getValidatedInteger(int* output, int minVal, int maxVal) {
    if (output == NULL) return false;
    
    char buffer[MAX_INT_DIGITS + 3]; // +1 for minus, +1 for newline, +1 for null
    
    // Initialize buffer to prevent undefined behavior
    memset(buffer, 0, sizeof(buffer));
    
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false;
    }
    
    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    // Check for empty input
    if (len == 0) {
        return false;
    }
    
    // Check length bounds
    if (len > MAX_INT_DIGITS) {
        return false;
    }
    
    // Validate that string contains only digits and optional leading minus
    size_t start = 0;
    if (buffer[0] == '-') {
        if (len == 1) return false;
        start = 1;
    }
    
    for (size_t i = start; i < len; i++) {
        if (!isdigit((unsigned char)buffer[i])) {
            return false;
        }
    }
    
    // Convert to integer with overflow checking
    errno = 0;
    char* endptr = NULL;
    long value = strtol(buffer, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || endptr == buffer || *endptr != '\\0') {
        return false;
    }
    
    // Check for overflow to int type
    if (value < INT_MIN || value > INT_MAX) {
        return false;
    }
    
    int intValue = (int)value;
    
    // Validate range
    if (!validateInteger(intValue, minVal, maxVal)) {
        return false;
    }
    
    *output = intValue;
    return true;
}

// Safe string input function with validation
bool getValidatedString(char* output, size_t outputSize, size_t minLen, size_t maxLen) {
    if (output == NULL || outputSize == 0) return false;
    
    // Initialize output buffer
    memset(output, 0, outputSize);
    
    // Ensure buffer is large enough for maxLen + null terminator
    if (outputSize <= maxLen) {
        return false;
    }
    
    char buffer[MAX_INPUT_LENGTH + 2]; // +1 for newline, +1 for null
    memset(buffer, 0, sizeof(buffer));
    
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false;
    }
    
    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    // Validate length
    if (len < minLen || len > maxLen) {
        return false;
    }
    
    // Validate content
    if (!validateAlphanumeric(buffer)) {
        return false;
    }
    
    // Safe copy with bounds check
    if (len >= outputSize) {
        return false;
    }
    
    strncpy(output, buffer, outputSize - 1);
    output[outputSize - 1] = '\\0'; // Ensure null termination
    
    return true;
}

// Performs operations on validated input
char* processInput(const char* text, int operation, char* result, size_t resultSize) {
    if (text == NULL || result == NULL || resultSize == 0) {
        return NULL;
    }
    
    // Initialize result buffer
    memset(result, 0, resultSize);
    
    size_t textLen = strlen(text);
    
    switch (operation) {
        case 1: // Convert to uppercase
            if (textLen >= resultSize) return NULL;
            for (size_t i = 0; i < textLen && i < resultSize - 1; i++) {
                result[i] = (char)toupper((unsigned char)text[i]);
            }
            result[textLen] = '\\0';
            break;
            
        case 2: // Convert to lowercase
            if (textLen >= resultSize) return NULL;
            for (size_t i = 0; i < textLen && i < resultSize - 1; i++) {
                result[i] = (char)tolower((unsigned char)text[i]);
            }
            result[textLen] = '\\0';
            break;
            
        case 3: // Count characters
            snprintf(result, resultSize, "Character count: %zu", textLen);
            break;
            
        default:
            snprintf(result, resultSize, "Invalid operation");
            break;
    }
    
    return result;
}

int main(void) {
    printf("=== Secure Input Processing Program ===\\n");
    
    // Test case 1: Valid uppercase conversion
    printf("\\nTest 1: Uppercase conversion\\n");
    {
        const char* text = "Hello World 123";
        int op = 1;
        char result[MAX_INPUT_LENGTH + 1];
        memset(result, 0, sizeof(result));
        
        if (validateAlphanumeric(text) && validateStringLength(text, MIN_INPUT_LENGTH, MAX_INPUT_LENGTH)) {
            printf("Input: %s\\n", text);
            if (processInput(text, op, result, sizeof(result)) != NULL) {
                printf("Output: %s\\n", result);
            }
        }
    }
    
    // Test case 2: Valid lowercase conversion
    printf("\\nTest 2: Lowercase conversion\\n");
    {
        const char* text = "TESTING ABC 456";
        int op = 2;
        char result[MAX_INPUT_LENGTH + 1];
        memset(result, 0, sizeof(result));
        
        if (validateAlphanumeric(text) && validateStringLength(text, MIN_INPUT_LENGTH, MAX_INPUT_LENGTH)) {
            printf("Input: %s\\n", text);
            if (processInput(text, op, result, sizeof(result)) != NULL) {
                printf("Output: %s\\n", result);
            }
        }
    }
    
    // Test case 3: Character count
    printf("\\nTest 3: Character count\\n");
    {
        const char* text = "Secure Code";
        int op = 3;
        char result[MAX_INPUT_LENGTH + 1];
        memset(result, 0, sizeof(result));
        
        if (validateAlphanumeric(text) && validateStringLength(text, MIN_INPUT_LENGTH, MAX_INPUT_LENGTH)) {
            printf("Input: %s\\n", text);
            if (processInput(text, op, result, sizeof(result)) != NULL) {
                printf("Output: %s\\n", result);
            }
        }
    }
    
    // Test case 4: Invalid input - special characters rejected
    printf("\\nTest 4: Invalid input with special characters\\n");
    {
        const char* text = "Hello@World!";
        if (!validateAlphanumeric(text)) {
            printf("Input rejected: Contains invalid characters\\n");
        }
    }
    
    // Test case 5: Invalid input - exceeds length
    printf("\\nTest 5: Invalid input exceeds maximum length\\n");
    {
        char text[MAX_INPUT_LENGTH + 51];
        memset(text, 'A', sizeof(text) - 1);
        text[sizeof(text) - 1] = '\\0';
        
        if (!validateStringLength(text, MIN_INPUT_LENGTH, MAX_INPUT_LENGTH)) {
            printf("Input rejected: Exceeds maximum length of %d characters\\n", MAX_INPUT_LENGTH);
        }
    }
    
    return 0;
}
