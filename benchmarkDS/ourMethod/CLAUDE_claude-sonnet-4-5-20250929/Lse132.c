
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_TEMPLATE_SIZE 4096
#define MAX_OUTPUT_SIZE 8192

// HTML escape function to prevent XSS attacks
// Returns allocated string that must be freed by caller
char* escapeHtml(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    if (input_len > MAX_USERNAME_LENGTH) {
        return NULL;
    }
    
    // Worst case: every character becomes 6 characters (&quot;)
    size_t max_output_len = input_len * 6 + 1;
    char* output = (char*)calloc(max_output_len, sizeof(char));
    
    if (output == NULL) {
        return NULL; // Allocation failed
    }
    
    size_t out_idx = 0;
    
    for (size_t i = 0; i < input_len && out_idx < max_output_len - 7; i++) {
        switch (input[i]) {
            case '&':
                memcpy(output + out_idx, "&amp;", 5);
                out_idx += 5;
                break;
            case '<':
                memcpy(output + out_idx, "&lt;", 4);
                out_idx += 4;
                break;
            case '>':
                memcpy(output + out_idx, "&gt;", 4);
                out_idx += 4;
                break;
            case '"':
                memcpy(output + out_idx, "&quot;", 6);
                out_idx += 6;
                break;
            case '\\'':
                memcpy(output + out_idx, "&#x27;", 6);
                out_idx += 6;
                break;
            case '/':
                memcpy(output + out_idx, "&#x2F;", 6);
                out_idx += 6;
                break;
            default:
                output[out_idx++] = input[i];
                break;
        }
    }
    
    output[out_idx] = '\\0';
    return output;
}

// Validate username against whitelist pattern
// Only allows alphanumeric characters and underscore
bool isValidUsername(const char* username) {
    if (username == NULL) {
        return false;
    }
    
    size_t len = strlen(username);
    
    // Check length constraints
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Whitelist validation - only alphanumeric and underscore
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum((unsigned char)c) && c != '_') {
            return false;
        }
    }
    
    return true;
}

// Render template with secure variable substitution
// Returns allocated string that must be freed by caller
char* renderTemplate(const char* username) {
    // Validate username before processing
    if (!isValidUsername(username)) {
        return NULL; // Invalid username
    }
    
    // Escape username to prevent XSS
    char* safeUsername = escapeHtml(username);
    if (safeUsername == NULL) {
        return NULL;
    }
    
    // Template content
    const char* templateContent = 
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head>\\n"
        "    <meta charset=\\"UTF-8\\">\\n"
        "    <meta http-equiv=\\"X-Content-Type-Options\\" content=\\"nosniff\\">\\n"
        "    <meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'\\">\\n"
        "    <title>Hello</title>\\n"
        "</head>\\n"
        "<body>\\n"
        "    <h1>Hello, {{ username }}!</h1>\\n"
        "    <p>Welcome to the page.</p>\\n"
        "</body>\\n"
        "</html>";
    
    // Allocate output buffer
    char* output = (char*)calloc(MAX_OUTPUT_SIZE, sizeof(char));
    if (output == NULL) {
        free(safeUsername);
        return NULL;
    }
    
    // Find and replace placeholder
    const char* placeholder = "{{ username }}";
    const char* pos = strstr(templateContent, placeholder);
    
    if (pos != NULL) {
        size_t prefix_len = pos - templateContent;
        size_t suffix_start = prefix_len + strlen(placeholder);
        size_t safe_username_len = strlen(safeUsername);
        
        // Check buffer capacity before writing
        if (prefix_len + safe_username_len + strlen(templateContent + suffix_start) >= MAX_OUTPUT_SIZE) {
            free(safeUsername);
            free(output);
            return NULL;
        }
        
        // Copy prefix
        memcpy(output, templateContent, prefix_len);
        
        // Copy escaped username
        memcpy(output + prefix_len, safeUsername, safe_username_len);
        
        // Copy suffix
        strcpy(output + prefix_len + safe_username_len, templateContent + suffix_start);
    } else {
        // No placeholder found, copy template as-is
        strncpy(output, templateContent, MAX_OUTPUT_SIZE - 1);
        output[MAX_OUTPUT_SIZE - 1] = '\\0';
    }
    
    free(safeUsername);
    return output;
}

int main(void) {
    // Test Case 1 - Valid alphanumeric username
    printf("Test Case 1 - Valid alphanumeric username:\\n");
    char* result1 = renderTemplate("john123");
    if (result1 != NULL) {
        printf("%.100s...\\n\\n", result1);
        free(result1);
    } else {
        printf("Rejected: Invalid username\\n\\n");
    }
    
    // Test Case 2 - Valid username with underscore
    printf("Test Case 2 - Valid username with underscore:\\n");
    char* result2 = renderTemplate("user_name");
    if (result2 != NULL) {
        printf("%.100s...\\n\\n", result2);
        free(result2);
    } else {
        printf("Rejected: Invalid username\\n\\n");
    }
    
    // Test Case 3 - Simple username
    printf("Test Case 3 - Simple username:\\n");
    char* result3 = renderTemplate("alice");
    if (result3 != NULL) {
        printf("%.100s...\\n\\n", result3);
        free(result3);
    } else {
        printf("Rejected: Invalid username\\n\\n");
    }
    
    // Test Case 4 - XSS attempt (should be rejected)
    printf("Test Case 4 - XSS attempt (should be rejected):\\n");
    char* result4 = renderTemplate("<script>alert('xss')</script>");
    if (result4 != NULL) {
        printf("%.100s...\\n\\n", result4);
        free(result4);
    } else {
        printf("Rejected: Invalid username\\n\\n");
    }
    
    // Test Case 5 - Path traversal attempt (should be rejected)
    printf("Test Case 5 - Path traversal attempt (should be rejected):\\n");
    char* result5 = renderTemplate("../etc/passwd");
    if (result5 != NULL) {
        printf("%.100s...\\n\\n", result5);
        free(result5);
    } else {
        printf("Rejected: Invalid username\\n\\n");
    }
    
    return 0;
}
