
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 50
#define MAX_INPUT_LEN 1000
#define MAX_OUTPUT_LEN 2048

// Validate username against whitelist - only alphanumeric, hyphen, underscore
int is_valid_username(const char* username) {
    if (username == NULL) {
        return 0;
    }
    
    size_t len = strlen(username);
    
    // Check length bounds
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    // Check each character against whitelist
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_') {
            return 0;
        }
    }
    
    return 1;
}

// HTML escape function to prevent XSS - returns newly allocated string
char* escape(const char* input) {
    if (input == NULL) {
        char* result = (char*)malloc(1);
        if (result != NULL) {
            result[0] = '\\0';
        }
        return result;
    }
    
    size_t input_len = strlen(input);
    
    // Validate input length to prevent DoS
    if (input_len == 0 || input_len > MAX_INPUT_LEN) {
        char* result = (char*)malloc(1);
        if (result != NULL) {
            result[0] = '\\0';
        }
        return result;
    }
    
    // Calculate maximum possible escaped length
    size_t max_escaped_len = input_len * 6 + 1; // Each char could become &#x27; (6 chars)
    if (max_escaped_len > MAX_OUTPUT_LEN) {
        max_escaped_len = MAX_OUTPUT_LEN;
    }
    
    char* escaped = (char*)malloc(max_escaped_len);
    if (escaped == NULL) {
        return NULL;
    }
    
    size_t pos = 0;
    for (size_t i = 0; i < input_len && pos < max_escaped_len - 7; i++) {
        char c = input[i];
        switch (c) {
            case '<':
                if (pos + 4 < max_escaped_len) {
                    memcpy(escaped + pos, "&lt;", 4);
                    pos += 4;
                }
                break;
            case '>':
                if (pos + 4 < max_escaped_len) {
                    memcpy(escaped + pos, "&gt;", 4);
                    pos += 4;
                }
                break;
            case '&':
                if (pos + 5 < max_escaped_len) {
                    memcpy(escaped + pos, "&amp;", 5);
                    pos += 5;
                }
                break;
            case '"':
                if (pos + 6 < max_escaped_len) {
                    memcpy(escaped + pos, "&quot;", 6);
                    pos += 6;
                }
                break;
            case '\\'':
                if (pos + 6 < max_escaped_len) {
                    memcpy(escaped + pos, "&#x27;", 6);
                    pos += 6;
                }
                break;
            case '/':
                if (pos + 6 < max_escaped_len) {
                    memcpy(escaped + pos, "&#x2F;", 6);
                    pos += 6;
                }
                break;
            default:
                if (pos < max_escaped_len - 1) {
                    escaped[pos++] = c;
                }
        }
    }
    
    escaped[pos] = '\\0';
    return escaped;
}

// Hello function that returns greeting with escaped username
char* hello(const char* username) {
    // Allocate buffer for result
    char* result = (char*)malloc(MAX_OUTPUT_LEN);
    if (result == NULL) {
        return NULL;
    }
    
    // Input validation - reject null or invalid
    if (username == NULL || !is_valid_username(username)) {
        strncpy(result, "Hello Guest", MAX_OUTPUT_LEN - 1);
        result[MAX_OUTPUT_LEN - 1] = '\\0';
        return result;
    }
    
    // Escape the username to prevent XSS
    char* escaped_username = escape(username);
    if (escaped_username == NULL) {
        strncpy(result, "Hello Guest", MAX_OUTPUT_LEN - 1);
        result[MAX_OUTPUT_LEN - 1] = '\\0';
        return result;
    }
    
    // Safely construct result string with bounds checking
    int written = snprintf(result, MAX_OUTPUT_LEN, "Hello %s", escaped_username);
    
    // Free escaped username
    free(escaped_username);
    
    // Check for truncation
    if (written < 0 || written >= MAX_OUTPUT_LEN) {
        strncpy(result, "Hello Guest", MAX_OUTPUT_LEN - 1);
        result[MAX_OUTPUT_LEN - 1] = '\\0';
    }
    
    return result;
}

int main(void) {
    char* result = NULL;
    
    // Test case 1
    result = hello("john");
    if (result != NULL) {
        printf("Test 1: %s\\n", result);
        free(result);
    }
    
    // Test case 2
    result = hello("alice_123");
    if (result != NULL) {
        printf("Test 2: %s\\n", result);
        free(result);
    }
    
    // Test case 3
    result = hello("<script>alert('xss')</script>");
    if (result != NULL) {
        printf("Test 3: %s\\n", result);
        free(result);
    }
    
    // Test case 4
    result = hello("user-name");
    if (result != NULL) {
        printf("Test 4: %s\\n", result);
        free(result);
    }
    
    // Test case 5
    result = hello(NULL);
    if (result != NULL) {
        printf("Test 5: %s\\n", result);
        free(result);
    }
    
    return 0;
}
