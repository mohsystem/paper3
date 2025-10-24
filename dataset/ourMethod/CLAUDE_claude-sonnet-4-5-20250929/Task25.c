
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

// Security: Define maximum limits to prevent resource exhaustion
#define MAX_LINES 10000000
#define MAX_LINE_LENGTH 1000000
#define MAX_NUMBER_LENGTH 20

// Function to add line numbers to an array of strings
// Security: All inputs validated, all allocations checked, all memory freed on error
// Parameters: lines - array of string pointers, count - number of lines
// Returns: newly allocated array of numbered strings (caller must free)
char** number(char** lines, size_t count, size_t* result_count) {
    // Security: Validate input pointer for NULL
    if (lines == NULL && count > 0) {
        *result_count = 0;
        return NULL;
    }
    
    // Security: Validate result_count pointer
    if (result_count == NULL) {
        return NULL;
    }
    
    *result_count = count;
    
    // Security: Handle empty input
    if (count == 0) {
        return NULL;
    }
    
    // Security: Prevent integer overflow and resource exhaustion
    if (count > MAX_LINES) {
        *result_count = 0;
        return NULL;
    }
    
    // Security: Check for multiplication overflow before allocation
    if (count > SIZE_MAX / sizeof(char*)) {
        *result_count = 0;
        return NULL;
    }
    
    // Security: Allocate result array, check for NULL
    char** result = (char**)calloc(count, sizeof(char*));
    if (result == NULL) {
        *result_count = 0;
        return NULL;
    }
    
    // Process each line
    for (size_t i = 0; i < count; i++) {
        // Security: Validate that lines[i] is not NULL
        if (lines[i] == NULL) {
            // Clean up previously allocated strings
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Validate line length to prevent memory exhaustion
        size_t line_len = strlen(lines[i]);
        if (line_len > MAX_LINE_LENGTH) {
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Check for integer overflow when computing line number
        if (i >= (size_t)(INT_MAX - 1)) {
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Calculate required buffer size safely
        // Format: "number: line\\0"
        // Number length + ": " (2 chars) + line length + null terminator
        size_t needed = MAX_NUMBER_LENGTH + 2 + line_len + 1;
        
        // Security: Check for size overflow
        if (needed > MAX_LINE_LENGTH + MAX_NUMBER_LENGTH + 3) {
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Allocate buffer for numbered line, check for NULL
        result[i] = (char*)malloc(needed);
        if (result[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Use snprintf with explicit size to prevent buffer overflow
        // Returns number of characters that would be written (excluding null)
        int written = snprintf(result[i], needed, "%zu: %s", i + 1, lines[i]);
        
        // Security: Check for snprintf error or truncation
        if (written < 0 || (size_t)written >= needed) {
            for (size_t j = 0; j <= i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        // Security: Ensure null termination (snprintf does this, but explicit check)
        result[i][needed - 1] = '\\0';
    }
    
    return result;
}

// Helper function to free the result array
void free_numbered_lines(char** lines, size_t count) {
    if (lines == NULL) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
}

int main(void) {
    // Test case 1: Empty list
    printf("Test 1 (empty): ");
    size_t count1 = 0;
    char** result1 = number(NULL, 0, &count1);
    printf("%s\\n", (result1 == NULL && count1 == 0) ? "PASS" : "FAIL");
    
    // Test case 2: Basic example ["a", "b", "c"]
    printf("Test 2 (a,b,c): ");
    char* test2[] = {"a", "b", "c"};
    size_t count2 = 0;
    char** result2 = number(test2, 3, &count2);
    if (result2 != NULL) {
        for (size_t i = 0; i < count2; i++) {
            printf("%s ", result2[i]);
        }
        printf("\\n");
        free_numbered_lines(result2, count2);
    }
    
    // Test case 3: Single line
    printf("Test 3 (single): ");
    char* test3[] = {"hello world"};
    size_t count3 = 0;
    char** result3 = number(test3, 1, &count3);
    if (result3 != NULL) {
        printf("%s\\n", result3[0]);
        free_numbered_lines(result3, count3);
    }
    
    // Test case 4: Lines with special characters
    printf("Test 4 (special chars): ");
    char* test4[] = {"line!@#", "line$%^", "line&*()"};
    size_t count4 = 0;
    char** result4 = number(test4, 3, &count4);
    if (result4 != NULL) {
        for (size_t i = 0; i < count4; i++) {
            printf("%s ", result4[i]);
        }
        printf("\\n");
        free_numbered_lines(result4, count4);
    }
    
    // Test case 5: Multiple lines
    printf("Test 5 (five lines): ");
    char* test5[] = {"first", "second", "third", "fourth", "fifth"};
    size_t count5 = 0;
    char** result5 = number(test5, 5, &count5);
    if (result5 != NULL) {
        for (size_t i = 0; i < count5; i++) {
            printf("%s ", result5[i]);
        }
        printf("\\n");
        free_numbered_lines(result5, count5);
    }
    
    return 0;
}
