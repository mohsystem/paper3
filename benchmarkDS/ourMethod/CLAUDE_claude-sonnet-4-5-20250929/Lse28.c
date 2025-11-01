
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

// Default URL is a constant, not constructed from user input
#define DEFAULT_URL "https://example.com/default"
#define MAX_URL_LENGTH 2048

// Global regex compiled once at program start
static regex_t url_regex;
static int regex_initialized = 0;

/**
 * Initialize regex pattern at first use.
 * Pattern validates HTTPS URLs with safe character set.
 */
static void init_regex(void) {
    if (!regex_initialized) {
        // Compile regex with extended syntax
        int ret = regcomp(&url_regex, "^https://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]\\\\{2,\\\\}/[a-zA-Z0-9/_-]*$", REG_EXTENDED | REG_NOSUB);
        if (ret != 0) {
            fprintf(stderr, "Failed to compile regex\\n");
            exit(1);
        }
        regex_initialized = 1;
    }
}

/**
 * Validates and returns a redirect URL.
 * The regex validates format only; matched URL is used directly without reconstruction.
 * 
 * @param target_url The URL to validate (must not be NULL)
 * @param result_buffer Buffer to store result (must be at least MAX_URL_LENGTH + 1 bytes)
 */
void get_redirect_url(const char* target_url, char* result_buffer) {
    // Initialize regex if not done yet
    init_regex();
    
    // Input validation: check for NULL pointer
    if (target_url == NULL || result_buffer == NULL) {
        strncpy(result_buffer, DEFAULT_URL, MAX_URL_LENGTH);
        result_buffer[MAX_URL_LENGTH] = '\\0';
        return;
    }
    
    // Check length to prevent buffer overflow
    size_t len = strnlen(target_url, MAX_URL_LENGTH + 1);
    if (len == 0 || len > MAX_URL_LENGTH) {
        strncpy(result_buffer, DEFAULT_URL, MAX_URL_LENGTH);
        result_buffer[MAX_URL_LENGTH] = '\\0';
        return;
    }
    
    // Validate against whitelist pattern
    // The regex is used only for validation, not construction
    int ret = regexec(&url_regex, target_url, 0, NULL, 0);
    
    // If pattern matches, copy the original validated input
    // Not constructing URL from regex groups - using validated input directly
    if (ret == 0) {
        strncpy(result_buffer, target_url, MAX_URL_LENGTH);
        result_buffer[MAX_URL_LENGTH] = '\\0';
    } else {
        // If validation fails, use safe default
        strncpy(result_buffer, DEFAULT_URL, MAX_URL_LENGTH);
        result_buffer[MAX_URL_LENGTH] = '\\0';
    }
}

int main(void) {
    char result[MAX_URL_LENGTH + 1];
    
    // Test case 1: Valid HTTPS URL
    get_redirect_url("https://trusted-site.com/page", result);
    printf("Test 1: %s\\n", result);
    
    // Test case 2: Valid HTTPS URL with path
    get_redirect_url("https://secure.example.org/user/profile", result);
    printf("Test 2: %s\\n", result);
    
    // Test case 3: Invalid - HTTP instead of HTTPS
    get_redirect_url("http://untrusted.com/page", result);
    printf("Test 3: %s\\n", result);
    
    // Test case 4: Invalid - contains special characters
    get_redirect_url("https://evil.com/redirect?url=javascript:alert(1)", result);
    printf("Test 4: %s\\n", result);
    
    // Test case 5: NULL input
    get_redirect_url(NULL, result);
    printf("Test 5: %s\\n", result);
    
    // Clean up regex
    if (regex_initialized) {
        regfree(&url_regex);
    }
    
    return 0;
}
