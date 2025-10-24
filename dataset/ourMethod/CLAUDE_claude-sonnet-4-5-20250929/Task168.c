
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Security: Maximum allowed string length to prevent excessive memory usage
#define MAX_STRING_LENGTH 1000000

// Security: Function to tweak letters with comprehensive input validation
// Returns: dynamically allocated string (caller must free), or NULL on error
// - All inputs validated for null pointers, bounds, and valid characters
// - Memory safely allocated and freed on error paths
// - Integer overflow checks before arithmetic operations
char* tweakLetters(const char* str, const int* tweaks, size_t length) {
    // Security: Validate input pointer is not NULL
    if (str == NULL) {
        fprintf(stderr, "Error: Input string is NULL\\n");
        return NULL;
    }
    
    // Security: Validate tweaks array is not NULL
    if (tweaks == NULL) {
        fprintf(stderr, "Error: Tweaks array is NULL\\n");
        return NULL;
    }
    
    // Security: Validate length is non-zero
    if (length == 0) {
        fprintf(stderr, "Error: Length cannot be zero\\n");
        return NULL;
    }
    
    // Security: Validate length doesn't exceed maximum to prevent excessive allocation\n    if (length > MAX_STRING_LENGTH) {\n        fprintf(stderr, "Error: String length exceeds maximum allowed\\n");\n        return NULL;\n    }\n    \n    // Security: Validate actual string length matches provided length\n    size_t actual_len = strnlen(str, length + 1);\n    if (actual_len != length) {\n        fprintf(stderr, "Error: String length mismatch\\n");\n        return NULL;\n    }\n    \n    // Security: Allocate memory with bounds check for overflow\n    // Check: length + 1 doesn't overflow
    if (length >= SIZE_MAX) {
        fprintf(stderr, "Error: Length overflow\\n");
        return NULL;
    }
    
    char* result = (char*)malloc(length + 1);
    // Security: Check malloc return value
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Security: Initialize all bytes including null terminator
    memset(result, 0, length + 1);
    
    // Process each character with bounds checking
    for (size_t i = 0; i < length; i++) {
        char c = str[i];
        int tweak = tweaks[i];
        
        // Security: Validate character is a lowercase letter
        if (c < 'a' || c > 'z') {
            fprintf(stderr, "Error: Invalid character at position %zu\\n", i);
            free(result); // Security: Free allocated memory before returning
            return NULL;
        }
        
        // Security: Validate tweak value is within reasonable bounds
        if (tweak < -26 || tweak > 26) {
            fprintf(stderr, "Error: Tweak value out of range at position %zu\\n", i);
            free(result);
            return NULL;
        }
        
        // Security: Safe modular arithmetic with overflow prevention
        int pos = (int)(c - 'a');
        
        // Security: Check for integer overflow before addition
        if ((tweak > 0 && pos > INT_MAX - tweak) || 
            (tweak < 0 && pos < INT_MIN - tweak)) {
            fprintf(stderr, "Error: Integer overflow at position %zu\\n", i);
            free(result);
            return NULL;
        }
        
        pos = (pos + tweak) % 26;
        
        // Security: Handle negative modulo results
        if (pos < 0) {
            pos += 26;
        }
        
        // Security: Final bounds check before assignment
        if (pos < 0 || pos > 25) {
            fprintf(stderr, "Error: Position out of bounds at %zu\\n", i);
            free(result);
            return NULL;
        }
        
        // Security: Bounds-checked array access
        result[i] = (char)('a' + pos);
    }
    
    // Security: Ensure null termination
    result[length] = '\\0';
    
    return result;
}

// Test cases
int main(void) {
    char* result = NULL;
    
    // Test case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    result = tweakLetters("apple", tweaks1, 5);
    if (result != NULL) {
        printf("Test 1: %s (expected: aqold)\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 2
    int tweaks2[] = {0, 0, 0, -1};
    result = tweakLetters("many", tweaks2, 4);
    if (result != NULL) {
        printf("Test 2: %s (expected: manx)\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    result = tweakLetters("rhino", tweaks3, 5);
    if (result != NULL) {
        printf("Test 3: %s (expected: sijop)\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 4: wrap-around from 'z'
    int tweaks4[] = {1, 1, 1};
    result = tweakLetters("xyz", tweaks4, 3);
    if (result != NULL) {
        printf("Test 4: %s (expected: yza)\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 5: wrap-around from 'a' backwards
    int tweaks5[] = {-1, -1, -1};
    result = tweakLetters("abc", tweaks5, 3);
    if (result != NULL) {
        printf("Test 5: %s (expected: zab)\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
