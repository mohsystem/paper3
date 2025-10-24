
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Maximum input length to prevent excessive memory allocation
#define MAX_INPUT_LENGTH 10000

// Function to convert string to Jaden Case
// Returns newly allocated string (caller must free) or NULL on error
// Security: Input validation, bounds checking, safe memory management
char* toJadenCase(const char* input) {
    // Validate input pointer
    if (input == NULL) {
        return NULL;
    }
    
    // Get input length with validation
    size_t inputLen = strlen(input);
    
    // Return NULL for empty string (as per specification)
    if (inputLen == 0) {
        return NULL;
    }
    
    // Validate length to prevent excessive memory allocation
    if (inputLen > MAX_INPUT_LENGTH) {
        fprintf(stderr, "Error: Input exceeds maximum length\\n");
        return NULL;
    }
    
    // Allocate memory for result (+1 for null terminator)
    // Check for potential overflow in allocation size
    if (inputLen >= SIZE_MAX - 1) {
        fprintf(stderr, "Error: Input length too large\\n");
        return NULL;
    }
    
    char* result = (char*)malloc(inputLen + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Copy input to result with bounds checking
    // Using strncpy to ensure we don't overflow\n    strncpy(result, input, inputLen);\n    result[inputLen] = '\\0'; // Ensure null termination\n    \n    // Flag to track if we're at the start of a word
    int newWord = 1;
    
    // Iterate through each character with explicit bounds checking
    for (size_t i = 0; i < inputLen; ++i) {
        // Additional bounds check (defensive programming)
        if (i >= inputLen) {
            break;
        }
        
        // Cast to unsigned char to avoid undefined behavior with negative values
        unsigned char c = (unsigned char)result[i];
        
        if (newWord && isalpha(c)) {
            // Capitalize first letter of word
            result[i] = (char)toupper(c);
            newWord = 0;
        } else if (isspace(c)) {
            // Space indicates start of new word
            newWord = 1;
        }
    }
    
    return result;
}

int main(void) {
    char* result = NULL;
    
    // Test case 1: Normal sentence
    const char* test1 = "How can mirrors be real if our eyes aren't real";
    result = toJadenCase(test1);
    printf("Test 1: %s\\n", result ? result : "NULL");
    free(result);
    result = NULL;
    
    // Test case 2: Empty string
    const char* test2 = "";
    result = toJadenCase(test2);
    printf("Test 2: %s\\n", result ? result : "NULL");
    free(result);
    result = NULL;
    
    // Test case 3: Single word
    const char* test3 = "hello";
    result = toJadenCase(test3);
    printf("Test 3: %s\\n", result ? result : "NULL");
    free(result);
    result = NULL;
    
    // Test case 4: Multiple spaces
    const char* test4 = "this  has   multiple spaces";
    result = toJadenCase(test4);
    printf("Test 4: %s\\n", result ? result : "NULL");
    free(result);
    result = NULL;
    
    // Test case 5: NULL input
    result = toJadenCase(NULL);
    printf("Test 5: %s\\n", result ? result : "NULL");
    free(result);
    result = NULL;
    
    return 0;
}
