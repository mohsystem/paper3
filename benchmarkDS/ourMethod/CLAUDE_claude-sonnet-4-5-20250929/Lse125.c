
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Trims trailing whitespace from a string in place.
 * Security measures:
 * - Null pointer checks
 * - Bounds checking on all array accesses
 * - No buffer overflows - operates on existing buffer
 * - Uses size_t for all indexing to prevent integer overflow
 * Returns: pointer to the input string (modified in place)
 */
char* trimTrailingWhitespace(char* input) {
    // Validate input pointer
    if (input == NULL) {
        return NULL;
    }
    
    // Get string length with bounds checking
    size_t len = strlen(input);
    
    // Handle empty string
    if (len == 0) {
        return input;
    }
    
    // Find the last non-whitespace character
    // Use size_t to prevent underflow, check > 0 before decrement
    size_t lastNonWhitespace = len;
    while (lastNonWhitespace > 0 && isspace((unsigned char)input[lastNonWhitespace - 1])) {
        lastNonWhitespace--;
    }
    
    // Null terminate at the correct position - bounds safe
    input[lastNonWhitespace] = '\\0';
    
    return input;
}

int main(int argc, char* argv[]) {
    // Test cases - using stack allocated buffers with known sizes
    char test1[] = "hello world   ";
    char test2[] = "no trailing";
    char test3[] = "   leading and trailing   ";
    char test4[] = "   ";
    char test5[] = "";
    
    printf("Test cases:\\n");
    
    // Test 1 - validate buffer before use
    char* result1 = trimTrailingWhitespace(test1);
    if (result1 != NULL) {
        printf("Test 1: [%s]\\n", result1);
    }
    
    // Test 2
    char* result2 = trimTrailingWhitespace(test2);
    if (result2 != NULL) {
        printf("Test 2: [%s]\\n", result2);
    }
    
    // Test 3
    char* result3 = trimTrailingWhitespace(test3);
    if (result3 != NULL) {
        printf("Test 3: [%s]\\n", result3);
    }
    
    // Test 4
    char* result4 = trimTrailingWhitespace(test4);
    if (result4 != NULL) {
        printf("Test 4: [%s]\\n", result4);
    }
    
    // Test 5
    char* result5 = trimTrailingWhitespace(test5);
    if (result5 != NULL) {
        printf("Test 5: [%s]\\n", result5);
    }
    
    // Process command line arguments if provided
    // Validate argc before accessing argv
    if (argc > 1) {
        printf("\\nCommand line argument processing:\\n");
        
        // Validate argv[1] is not null
        if (argv[1] != NULL) {
            // Validate input length to prevent resource exhaustion
            const size_t MAX_INPUT_LENGTH = 10000;
            size_t inputLength = strlen(argv[1]);
            
            if (inputLength > MAX_INPUT_LENGTH) {
                fprintf(stderr, "Error: Input exceeds maximum allowed length\\n");
                return 1;
            }
            
            // Allocate buffer with bounds check on malloc result
            // Add 1 for null terminator
            char* input = (char*)malloc(inputLength + 1);
            if (input == NULL) {
                fprintf(stderr, "Error: Memory allocation failed\\n");
                return 1;
            }
            
            // Safe string copy with explicit size limit
            strncpy(input, argv[1], inputLength);
            input[inputLength] = '\\0';  // Ensure null termination
            
            char* trimmed = trimTrailingWhitespace(input);
            if (trimmed != NULL) {
                printf("[%s]\\n", trimmed);
            }
            
            // Free allocated memory exactly once
            free(input);
            input = NULL;  // Prevent use after free
        }
    }
    
    return 0;
}
