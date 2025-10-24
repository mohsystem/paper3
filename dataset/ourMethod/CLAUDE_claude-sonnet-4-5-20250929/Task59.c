
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Security: Validate input and prevent buffer overflows
// Returns newly allocated string that caller must free, or NULL on error
char* accum(const char* s) {
    // Security: Validate input pointer
    if (s == NULL) {
        return NULL;
    }
    
    // Security: Validate input length to prevent integer overflow
    const size_t MAX_INPUT_LENGTH = 10000;
    size_t len = strlen(s);
    
    if (len == 0) {
        char* result = (char*)malloc(1);
        if (result == NULL) {
            return NULL;
        }
        result[0] = '\\0';
        return result;
    }
    
    if (len > MAX_INPUT_LENGTH) {
        return NULL;
    }
    
    // Security: Validate all characters are ASCII letters
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            return NULL;
        }
    }
    
    // Security: Calculate required buffer size to prevent overflow
    // Size = sum of (i+1) + separators + null terminator
    size_t required_size = (len * (len + 1)) / 2 + (len > 0 ? len - 1 : 0) + 1;
    
    // Security: Check for overflow and reasonable size limit
    if (required_size > 100000000 || required_size < len) {
        return NULL;
    }
    
    // Security: Allocate exact size needed
    char* result = (char*)malloc(required_size);
    if (result == NULL) {
        return NULL;
    }
    
    size_t pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        // Security: Bounds check before each write
        if (pos >= required_size - 1) {
            free(result);
            return NULL;
        }
        
        // Add separator after first character
        if (i > 0) {
            result[pos++] = '-';
        }
        
        // First occurrence: uppercase
        unsigned char c = (unsigned char)s[i];
        result[pos++] = (char)toupper(c);
        
        // Remaining occurrences: lowercase
        for (size_t j = 0; j < i; j++) {
            // Security: Bounds check
            if (pos >= required_size - 1) {
                free(result);
                return NULL;
            }
            result[pos++] = (char)tolower(c);
        }
    }
    
    // Security: Ensure null termination
    result[pos] = '\\0';
    
    return result;
}

int main() {
    char* result = NULL;
    
    // Test case 1
    result = accum("abcd");
    if (result != NULL) {
        printf("Test 1: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 2
    result = accum("RqaEzty");
    if (result != NULL) {
        printf("Test 2: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 3
    result = accum("cwAt");
    if (result != NULL) {
        printf("Test 3: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 4
    result = accum("ZpglnRxqenU");
    if (result != NULL) {
        printf("Test 4: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 5
    result = accum("a");
    if (result != NULL) {
        printf("Test 5: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
