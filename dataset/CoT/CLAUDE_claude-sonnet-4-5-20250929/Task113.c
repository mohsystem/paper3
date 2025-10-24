
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STRINGS 1000
#define MAX_STRING_LENGTH 1024

// Function to sanitize input string
char* sanitize_input(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    size_t len = strlen(input);
    char* sanitized = (char*)malloc(len + 1);
    
    if (sanitized == NULL) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        // Remove control characters while preserving valid input
        if ((input[i] >= 32 && input[i] <= 126) || 
            input[i] == '\\t' || input[i] == '\\n' || input[i] == '\\r') {
            sanitized[j++] = input[i];
        }
    }
    sanitized[j] = '\\0';
    
    return sanitized;
}

// Function to safely concatenate strings with input validation
char* concatenate_strings(char** strings, int count) {
    if (strings == NULL || count <= 0) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    // Calculate total length needed
    size_t total_length = 0;
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            total_length += strlen(strings[i]);
        }
    }
    
    // Allocate memory for result
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        return NULL;
    }
    
    result[0] = '\\0';
    
    // Concatenate all strings
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            char* sanitized = sanitize_input(strings[i]);
            if (sanitized != NULL) {
                strcat(result, sanitized);
                free(sanitized);
            }
        }
    }
    
    return result;
}

int main() {
    printf("=== Test Cases ===\\n\\n");
    
    // Test Case 1: Normal strings
    char* test1[] = {"Hello", " ", "World", "!"};
    char* result1 = concatenate_strings(test1, 4);
    printf("Test 1 - Normal strings: %s\\n", result1);
    free(result1);
    
    // Test Case 2: Empty array
    char* result2 = concatenate_strings(NULL, 0);
    printf("Test 2 - Empty list: '%s'\\n", result2);
    free(result2);
    
    // Test Case 3: Strings with numbers and special characters
    char* test3[] = {"User", "123", "@", "Example"};
    char* result3 = concatenate_strings(test3, 4);
    printf("Test 3 - Mixed content: %s\\n", result3);
    free(result3);
    
    // Test Case 4: Array with NULL values
    char* test4[] = {"Start", NULL, "End"};
    char* result4 = concatenate_strings(test4, 3);
    printf("Test 4 - With NULL values: %s\\n", result4);
    free(result4);
    
    // Test Case 5: Single string
    char* test5[] = {"SingleString"};
    char* result5 = concatenate_strings(test5, 1);
    printf("Test 5 - Single string: %s\\n", result5);
    free(result5);
    
    // Interactive input option
    printf("\\n=== Interactive Mode ===\\n");
    int count;
    printf("Enter number of strings to concatenate: ");
    
    if (scanf("%d", &count) != 1 || count < 0 || count > MAX_STRINGS) {
        printf("Invalid count. Please enter a value between 0 and %d.\\n", MAX_STRINGS);
        return 1;
    }
    
    getchar(); // Clear newline from buffer
    
    char** user_inputs = (char**)malloc(count * sizeof(char*));
    if (user_inputs == NULL) {
        printf("Memory allocation failed.\\n");
        return 1;
    }
    
    for (int i = 0; i < count; i++) {
        user_inputs[i] = (char*)malloc(MAX_STRING_LENGTH);
        if (user_inputs[i] == NULL) {
            printf("Memory allocation failed.\\n");
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(user_inputs[j]);
            }
            free(user_inputs);
            return 1;
        }
        
        printf("Enter string %d: ", i + 1);
        if (fgets(user_inputs[i], MAX_STRING_LENGTH, stdin) != NULL) {
            // Remove trailing newline
            size_t len = strlen(user_inputs[i]);
            if (len > 0 && user_inputs[i][len - 1] == '\\n') {
                user_inputs[i][len - 1] = '\\0';
            }
        }
    }
    
    char* result = concatenate_strings(user_inputs, count);
    printf("\\nConcatenated Result: %s\\n", result);
    
    // Free allocated memory
    free(result);
    for (int i = 0; i < count; i++) {
        free(user_inputs[i]);
    }
    free(user_inputs);
    
    return 0;
}
