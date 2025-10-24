
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_INPUT_LENGTH 50
#define MAX_PATH_LENGTH 100
#define NUM_VALID_PAGES 5

// Structure to hold allowlist mappings
typedef struct {
    const char* key;
    const char* value;
} PageMapping;

// Allowlist of valid page destinations to prevent open redirect vulnerabilities
static const PageMapping VALID_PAGES[NUM_VALID_PAGES] = {
    {"home", "/home.html"},
    {"about", "/about.html"},
    {"contact", "/contact.html"},
    {"services", "/services.html"},
    {"products", "/products.html"}
};

static const char* DEFAULT_PAGE = "/home.html";

// Validate and sanitize user input to prevent injection attacks
// Returns 1 on success, 0 on failure
int sanitizeInput(const char* input, char* output, size_t outputSize) {
    // Null pointer check
    if (input == NULL || output == NULL || outputSize == 0) {
        return 0;
    }
    
    // Initialize output buffer
    memset(output, 0, outputSize);
    
    size_t inputLen = strlen(input);
    
    // Input length validation - prevent excessive input
    if (inputLen == 0 || inputLen > MAX_INPUT_LENGTH) {
        return 0;
    }
    
    size_t outIdx = 0;
    
    // Only allow alphanumeric characters and underscores
    for (size_t i = 0; i < inputLen && outIdx < outputSize - 1; i++) {
        unsigned char c = (unsigned char)input[i];
        if (isalnum(c) || c == '_') {
            output[outIdx++] = (char)tolower(c);
        }
    }
    
    // Null-terminate the output
    output[outIdx] = '\\0';
    
    // Return failure if no valid characters found
    return (outIdx > 0) ? 1 : 0;
}

// Redirect user based on validated input using allowlist approach
// Returns pointer to redirect path (never returns NULL)
const char* redirectUser(const char* userInput) {
    char cleanInput[MAX_INPUT_LENGTH + 1];
    
    // Null pointer check
    if (userInput == NULL) {
        return DEFAULT_PAGE;
    }
    
    // Sanitize input to prevent injection attacks
    if (sanitizeInput(userInput, cleanInput, sizeof(cleanInput)) == 0) {
        return DEFAULT_PAGE;
    }
    
    // Use allowlist to validate destination - prevents open redirect (CWE-601)
    for (size_t i = 0; i < NUM_VALID_PAGES; i++) {
        if (strcmp(cleanInput, VALID_PAGES[i].key) == 0) {
            return VALID_PAGES[i].value;
        }
    }
    
    // Default to safe page if input not in allowlist
    return DEFAULT_PAGE;
}

int main(void) {
    // Test cases demonstrating secure redirect functionality
    const char* testCases[5] = {
        "home",                    // Valid input
        "about",                   // Valid input
        "products",                // Valid input
        "../etc/passwd",           // Path traversal attempt - blocked
        "javascript:alert(1)"      // XSS attempt - blocked
    };
    
    printf("Secure Page Redirect System\\n");
    printf("===========================\\n\\n");
    
    for (size_t i = 0; i < 5; i++) {
        const char* destination = redirectUser(testCases[i]);
        printf("Input: \\"%s\\" -> Redirect to: %s\\n", 
               testCases[i], destination);
    }
    
    return 0;
}
