
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_URL_LENGTH 2048
#define DOMAIN "example.com"

/**
 * Validates and extracts the domain from a URL
 * Returns true if domain matches expected domain, false otherwise
 * This prevents open redirect vulnerabilities by validating the domain
 */
bool is_valid_domain(const char *url) {
    // Rules#2: Input validation - check type, length, and format
    if (url == NULL) {
        return false;
    }
    
    // Rules#35: Validate size before processing
    size_t url_len = strnlen(url, MAX_URL_LENGTH + 1);
    if (url_len == 0 || url_len > MAX_URL_LENGTH) {
        return false;
    }
    
    // Rules#26: Normalize URL before validation
    // Check for common URL schemes
    const char *domain_start = NULL;
    
    if (strncmp(url, "http://", 7) == 0) {
        domain_start = url + 7;
    } else if (strncmp(url, "https://", 8) == 0) {
        domain_start = url + 8;
    } else {
        // No scheme found - invalid URL format
        return false;
    }
    
    // Rules#25: Use allowlist - only accept exact domain match
    // Find the end of the domain (before port, path, or query)
    size_t domain_len = strcspn(domain_start, ":/?#");
    
    // Check if domain matches exactly
    if (domain_len != strlen(DOMAIN)) {
        return false;
    }
    
    // Rules#34: Explicit bounds check before comparison
    if (strncmp(domain_start, DOMAIN, domain_len) == 0) {
        return true;
    }
    
    return false;
}

/**
 * Safely redirects to URL if domain is valid
 * Rules#1: Fail closed with clear error messages
 * Rules#11: Return generic messages, detailed info only to logs
 */
int redirect_invalid_url(const char *url, char *output_buffer, size_t buffer_size) {
    // Rules#28: Treat all input as untrusted
    if (url == NULL || output_buffer == NULL || buffer_size == 0) {
        return -1;
    }
    
    // Rules#36: Initialize buffer
    memset(output_buffer, 0, buffer_size);
    
    // Validate domain before redirecting
    if (is_valid_domain(url)) {
        // Rules#37: Use safe library functions with size limits
        int written = snprintf(output_buffer, buffer_size, 
                              "Redirect to: %s", url);
        
        // Check for truncation
        if (written < 0 || (size_t)written >= buffer_size) {
            return -1;
        }
        return 0; // Success
    } else {
        // Rules#11: Generic error message, no internal details leaked
        int written = snprintf(output_buffer, buffer_size, 
                              "Error: Invalid URL domain");
        
        if (written < 0 || (size_t)written >= buffer_size) {
            return -1;
        }
        return 1; // Error
    }
}

int main(void) {
    char output[512];
    int result;
    
    printf("Test Case 1: Valid URL with http://example.com\\n");
    result = redirect_invalid_url("http://example.com/page", output, sizeof(output));
    printf("Result: %d, Output: %s\\n\\n", result, output);
    
    printf("Test Case 2: Valid URL with https://example.com\\n");
    result = redirect_invalid_url("https://example.com/secure", output, sizeof(output));
    printf("Result: %d, Output: %s\\n\\n", result, output);
    
    printf("Test Case 3: Invalid domain (malicious.com)\\n");
    result = redirect_invalid_url("https://malicious.com/phishing", output, sizeof(output));
    printf("Result: %d, Output: %s\\n\\n", result, output);
    
    printf("Test Case 4: Invalid subdomain attack\\n");
    result = redirect_invalid_url("https://example.com.evil.com/attack", output, sizeof(output));
    printf("Result: %d, Output: %s\\n\\n", result, output);
    
    printf("Test Case 5: NULL input\\n");
    result = redirect_invalid_url(NULL, output, sizeof(output));
    printf("Result: %d, Output: %s\\n\\n", result, output);
    
    return 0;
}
