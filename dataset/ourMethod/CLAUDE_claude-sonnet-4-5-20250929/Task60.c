
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

// Maximum length for result string to prevent unbounded allocation
#define MAX_RESULT_LENGTH 10000
#define MAX_DIGITS 20

// Function to safely convert number to expanded form
// Returns allocated string that must be freed by caller, or NULL on error
// Security: All inputs validated, all allocations checked, all operations bounds-checked
char* expandedForm(long long num) {
    // Input validation: number must be positive as per requirements
    if (num <= 0) {
        return NULL;
    }
    
    // Convert number to string safely using snprintf with bounds
    char numStr[MAX_DIGITS + 1];
    int written = snprintf(numStr, sizeof(numStr), "%lld", num);
    
    // Validate snprintf success and no truncation
    if (written < 0 || written >= (int)sizeof(numStr)) {
        return NULL;
    }
    
    // Null-terminate explicitly (defense in depth)
    numStr[MAX_DIGITS] = '\\0';
    
    size_t numLen = strlen(numStr);
    
    // Validate string length
    if (numLen == 0 || numLen > MAX_DIGITS) {
        return NULL;
    }
    
    // Allocate result buffer with safety margin
    // Calculate maximum possible result size
    size_t maxSize = numLen * (MAX_DIGITS + 4); // each digit + zeros + " + "
    if (maxSize > MAX_RESULT_LENGTH) {
        maxSize = MAX_RESULT_LENGTH;
    }
    
    char* result = (char*)calloc(maxSize, sizeof(char));
    if (result == NULL) {
        return NULL; // Allocation failed
    }
    
    // Initialize result to empty string
    result[0] = '\\0';
    
    size_t resultPos = 0;
    bool firstPart = true;
    
    // Process each digit with bounds checking
    for (size_t i = 0; i < numLen; i++) {
        // Bounds check on array access
        if (i >= sizeof(numStr) - 1) {
            free(result);
            return NULL;
        }
        
        char digit = numStr[i];
        
        // Validate digit character
        if (!isdigit((unsigned char)digit)) {
            free(result);
            return NULL;
        }
        
        // Skip zeros
        if (digit == '0') {
            continue;
        }
        
        // Add separator if not first part
        if (!firstPart) {
            // Check bounds before adding separator
            if (resultPos + 3 >= maxSize) {
                free(result);
                return NULL;
            }
            result[resultPos++] = ' ';
            result[resultPos++] = '+';
            result[resultPos++] = ' ';
        }
        firstPart = false;
        
        // Add digit
        if (resultPos >= maxSize - 1) {
            free(result);
            return NULL;
        }
        result[resultPos++] = digit;
        
        // Add zeros for place value
        size_t zerosCount = numLen - i - 1;
        
        // Bounds check for zeros
        if (resultPos + zerosCount >= maxSize) {
            free(result);
            return NULL;
        }
        
        for (size_t j = 0; j < zerosCount; j++) {
            result[resultPos++] = '0';
        }
    }
    
    // Ensure null termination
    if (resultPos < maxSize) {
        result[resultPos] = '\\0';
    } else {
        free(result);
        return NULL;
    }
    
    return result;
}

int main(void) {
    // Test cases with proper memory management
    char* result1 = expandedForm(12);
    if (result1 != NULL) {
        printf("Test 1: %s\\n", result1);
        printf("Expected: 10 + 2\\n\\n");
        free(result1); // Free allocated memory
    }
    
    char* result2 = expandedForm(42);
    if (result2 != NULL) {
        printf("Test 2: %s\\n", result2);
        printf("Expected: 40 + 2\\n\\n");
        free(result2);
    }
    
    char* result3 = expandedForm(70304);
    if (result3 != NULL) {
        printf("Test 3: %s\\n", result3);
        printf("Expected: 70000 + 300 + 4\\n\\n");
        free(result3);
    }
    
    char* result4 = expandedForm(9);
    if (result4 != NULL) {
        printf("Test 4: %s\\n", result4);
        printf("Expected: 9\\n\\n");
        free(result4);
    }
    
    char* result5 = expandedForm(123456);
    if (result5 != NULL) {
        printf("Test 5: %s\\n", result5);
        printf("Expected: 100000 + 20000 + 3000 + 400 + 50 + 6\\n\\n");
        free(result5);
    }
    
    return 0;
}
