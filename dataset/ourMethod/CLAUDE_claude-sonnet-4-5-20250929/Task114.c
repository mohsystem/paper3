
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

// Security: Maximum string size to prevent excessive memory allocation
#define MAX_STRING_SIZE 1000000

// Security: All string operations validate inputs and check bounds
// Memory is properly allocated, checked for NULL, and freed
// No use of unsafe functions: gets, strcpy, strcat, sprintf, scanf without width

// Copy string with validation and safe allocation
// Returns NULL on failure, caller must free result
char* copyString(const char* input) {
    // Validate input is not NULL
    if (input == NULL) {
        fprintf(stderr, "Error: NULL input to copyString\\n");
        return NULL;
    }
    
    // Security: Validate string length with strnlen to prevent reading past buffer
    size_t len = strnlen(input, MAX_STRING_SIZE + 1);
    if (len > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Input string exceeds maximum size\\n");
        return NULL;
    }
    
    // Security: Check for allocation overflow
    if (len >= SIZE_MAX - 1) {
        fprintf(stderr, "Error: String length causes allocation overflow\\n");
        return NULL;
    }
    
    // Allocate memory with size validation
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Security: Use memcpy with known bounds instead of strcpy
    memcpy(result, input, len);
    result[len] = '\\0'; // Ensure null termination
    
    return result;
}

// Concatenate two strings with bounds checking
// Returns NULL on failure, caller must free result
char* concatenateStrings(const char* str1, const char* str2) {
    // Validate inputs are not NULL
    if (str1 == NULL || str2 == NULL) {
        fprintf(stderr, "Error: NULL input to concatenateStrings\\n");
        return NULL;
    }
    
    // Security: Validate lengths with strnlen
    size_t len1 = strnlen(str1, MAX_STRING_SIZE + 1);
    size_t len2 = strnlen(str2, MAX_STRING_SIZE + 1);
    
    if (len1 > MAX_STRING_SIZE || len2 > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Input string exceeds maximum size\\n");
        return NULL;
    }
    
    // Security: Check for addition overflow before allocation
    if (len1 > SIZE_MAX - len2 - 1) {
        fprintf(stderr, "Error: Concatenated length causes overflow\\n");
        return NULL;
    }
    
    size_t total_len = len1 + len2;
    if (total_len > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Concatenated string exceeds maximum size\\n");
        return NULL;
    }
    
    // Allocate memory for result
    char* result = (char*)malloc(total_len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Security: Use memcpy with known bounds
    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2);
    result[total_len] = '\\0'; // Ensure null termination
    
    return result;
}

// Reverse string with validation
// Returns NULL on failure, caller must free result
char* reverseString(const char* input) {
    if (input == NULL) {
        fprintf(stderr, "Error: NULL input to reverseString\\n");
        return NULL;
    }
    
    // Security: Validate length
    size_t len = strnlen(input, MAX_STRING_SIZE + 1);
    if (len > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Input string exceeds maximum size\\n");
        return NULL;
    }
    
    // Allocate memory for result
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Reverse the string with bounds checking
    for (size_t i = 0; i < len; i++) {
        result[i] = input[len - 1 - i];
    }
    result[len] = '\\0'; // Ensure null termination
    
    return result;
}

// Convert string to uppercase with validation
// Returns NULL on failure, caller must free result
char* toUpperCase(const char* input) {
    if (input == NULL) {
        fprintf(stderr, "Error: NULL input to toUpperCase\\n");
        return NULL;
    }
    
    // Security: Validate length
    size_t len = strnlen(input, MAX_STRING_SIZE + 1);
    if (len > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Input string exceeds maximum size\\n");
        return NULL;
    }
    
    // Allocate memory for result
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Convert to uppercase with bounds checking
    for (size_t i = 0; i < len; i++) {
        // Cast to unsigned char to avoid undefined behavior with negative values
        result[i] = (char)toupper((unsigned char)input[i]);
    }
    result[len] = '\\0'; // Ensure null termination
    
    return result;
}

// Find substring with bounds checking
// Returns position or -1 if not found
int findSubstring(const char* haystack, const char* needle) {
    if (haystack == NULL || needle == NULL) {
        fprintf(stderr, "Error: NULL input to findSubstring\\n");
        return -1;
    }
    
    // Security: Validate lengths
    size_t haystack_len = strnlen(haystack, MAX_STRING_SIZE + 1);
    size_t needle_len = strnlen(needle, MAX_STRING_SIZE + 1);
    
    if (haystack_len > MAX_STRING_SIZE || needle_len > MAX_STRING_SIZE) {
        fprintf(stderr, "Error: Input string exceeds maximum size\\n");
        return -1;
    }
    
    // Empty needle found at position 0
    if (needle_len == 0) {
        return 0;
    }
    
    // Needle longer than haystack, cannot be found
    if (needle_len > haystack_len) {
        return -1;
    }
    
    // Security: Bounds-checked search
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(haystack + i, needle, needle_len) == 0) {
            // Validate position fits in int before returning
            if (i > (size_t)INT_MAX) {
                fprintf(stderr, "Error: Position exceeds int range\\n");
                return -1;
            }
            return (int)i;
        }
    }
    
    return -1; // Not found
}

int main(void) {
    printf("String Operations Test Cases\\n");
    printf("============================\\n\\n");
    
    // Test Case 1: Copy operation
    printf("Test 1 - Copy String:\\n");
    const char* original = "Hello, World!";
    char* copied = copyString(original);
    if (copied != NULL) {
        printf("Original: %s\\n", original);
        printf("Copied: %s\\n\\n", copied);
        free(copied); // Security: Free allocated memory
        copied = NULL;
    }
    
    // Test Case 2: Concatenate operation
    printf("Test 2 - Concatenate Strings:\\n");
    const char* str1 = "Secure ";
    const char* str2 = "Coding";
    char* concatenated = concatenateStrings(str1, str2);
    if (concatenated != NULL) {
        printf("String 1: %s\\n", str1);
        printf("String 2: %s\\n", str2);
        printf("Concatenated: %s\\n\\n", concatenated);
        free(concatenated); // Security: Free allocated memory
        concatenated = NULL;
    }
    
    // Test Case 3: Reverse operation
    printf("Test 3 - Reverse String:\\n");
    const char* toReverse = "12345";
    char* reversed = reverseString(toReverse);
    if (reversed != NULL) {
        printf("Original: %s\\n", toReverse);
        printf("Reversed: %s\\n\\n", reversed);
        free(reversed); // Security: Free allocated memory
        reversed = NULL;
    }
    
    // Test Case 4: Uppercase operation
    printf("Test 4 - Convert to Uppercase:\\n");
    const char* lowercase = "testing uppercase";
    char* uppercase = toUpperCase(lowercase);
    if (uppercase != NULL) {
        printf("Original: %s\\n", lowercase);
        printf("Uppercase: %s\\n\\n", uppercase);
        free(uppercase); // Security: Free allocated memory
        uppercase = NULL;
    }
    
    // Test Case 5: Find substring operation
    printf("Test 5 - Find Substring:\\n");
    const char* haystack = "The quick brown fox jumps";
    const char* needle = "brown";
    int position = findSubstring(haystack, needle);
    printf("Haystack: %s\\n", haystack);
    printf("Needle: %s\\n", needle);
    printf("Position: %d\\n", position);
    
    return 0;
}
