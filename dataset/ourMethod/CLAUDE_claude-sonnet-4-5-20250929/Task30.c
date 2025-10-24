
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Function to validate that string contains only lowercase letters a-z
// Security: Prevents processing of malicious input with control characters
bool validate_input(const char* str) {
    if (str == NULL) {
        return false;
    }
    
    // Check string length to prevent excessive processing
    size_t len = strlen(str);
    if (len > 100000) { // Reasonable limit for string length
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        if (str[i] < 'a' || str[i] > 'z') {
            return false;
        }
    }
    return true;
}

// Function to merge two strings and return sorted distinct letters
// Returns NULL on error - caller must free returned string
char* longest(const char* s1, const char* s2) {
    // Validate inputs - NULL check prevents dereferencing NULL pointers
    if (s1 == NULL || s2 == NULL) {
        return NULL;
    }
    
    // Validate that strings contain only lowercase letters
    if (!validate_input(s1) || !validate_input(s2)) {
        return NULL;
    }
    
    // Use a boolean array to track presence of each letter (a-z)
    // Constant size allocation avoids variable-length arrays and overflow
    bool present[26] = {false};
    
    // Mark all characters from s1 as present
    size_t len1 = strlen(s1);
    for (size_t i = 0; i < len1; i++) {
        present[s1[i] - 'a'] = true;
    }
    
    // Mark all characters from s2 as present
    size_t len2 = strlen(s2);
    for (size_t i = 0; i < len2; i++) {
        present[s2[i] - 'a'] = true;
    }
    
    // Allocate result string (max 26 letters + null terminator)
    // Fixed size allocation prevents buffer overflow
    char* result = (char*)malloc(27 * sizeof(char));
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }
    
    // Initialize result to empty string
    result[0] = '\\0';
    
    // Build result string with sorted unique characters
    size_t result_idx = 0;
    for (int i = 0; i < 26; i++) {
        if (present[i]) {
            result[result_idx++] = 'a' + i;
        }
    }
    result[result_idx] = '\\0'; // Null-terminate the string
    
    return result;
}

int main(void) {
    // Test case 1
    char* result1 = longest("xyaabbbccccdefww", "xxxxyyyyabklmopq");
    if (result1 != NULL) {
        printf("Test 1: %s\\n", result1);
        printf("Expected: abcdefklmopqwxy\\n");
        free(result1); // Free allocated memory
        result1 = NULL; // Prevent double-free
    } else {
        printf("Test 1 failed: NULL returned\\n");
    }
    
    // Test case 2
    char* result2 = longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
    if (result2 != NULL) {
        printf("\\nTest 2: %s\\n", result2);
        printf("Expected: abcdefghijklmnopqrstuvwxyz\\n");
        free(result2);
        result2 = NULL;
    } else {
        printf("Test 2 failed: NULL returned\\n");
    }
    
    // Test case 3: Empty strings
    char* result3 = longest("", "");
    if (result3 != NULL) {
        printf("\\nTest 3: %s\\n", result3);
        printf("Expected: (empty)\\n");
        free(result3);
        result3 = NULL;
    } else {
        printf("Test 3 failed: NULL returned\\n");
    }
    
    // Test case 4: One empty string
    char* result4 = longest("abc", "");
    if (result4 != NULL) {
        printf("\\nTest 4: %s\\n", result4);
        printf("Expected: abc\\n");
        free(result4);
        result4 = NULL;
    } else {
        printf("Test 4 failed: NULL returned\\n");
    }
    
    // Test case 5: No common characters
    char* result5 = longest("abc", "def");
    if (result5 != NULL) {
        printf("\\nTest 5: %s\\n", result5);
        printf("Expected: abcdef\\n");
        free(result5);
        result5 = NULL;
    } else {
        printf("Test 5 failed: NULL returned\\n");
    }
    
    return 0;
}
