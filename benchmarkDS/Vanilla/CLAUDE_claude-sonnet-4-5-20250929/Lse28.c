
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

// Global regex variable - compiled once
static regex_t url_regex;
static int regex_compiled = 0;
static const char* DEFAULT_URL = "https://www.default.com";

// Initialize regex (called once)
void init_regex() {
    if (!regex_compiled) {
        regcomp(&url_regex, "^https?://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}(/.*)?$", REG_EXTENDED);
        regex_compiled = 1;
    }
}

const char* get_redirect_url(const char* target_url, char* result_buffer) {
    init_regex();
    
    // If the target URL matches the pattern, use it as redirect target
    if (regexec(&url_regex, target_url, 0, NULL, 0) == 0) {
        strcpy(result_buffer, target_url);
        return result_buffer;
    }
    
    // If the target URL does not match, use default URL
    strcpy(result_buffer, DEFAULT_URL);
    return result_buffer;
}

void cleanup_regex() {
    if (regex_compiled) {
        regfree(&url_regex);
        regex_compiled = 0;
    }
}

int main() {
    char result[256];
    
    // Test case 1: Valid HTTP URL
    printf("Test 1: %s\\n", get_redirect_url("http://www.example.com", result));
    
    // Test case 2: Valid HTTPS URL with path
    printf("Test 2: %s\\n", get_redirect_url("https://www.example.com/path/to/page", result));
    
    // Test case 3: Valid URL with subdomain
    printf("Test 3: %s\\n", get_redirect_url("https://subdomain.example.com", result));
    
    // Test case 4: Invalid URL (missing protocol)
    printf("Test 4: %s\\n", get_redirect_url("www.example.com", result));
    
    // Test case 5: Invalid URL (random text)
    printf("Test 5: %s\\n", get_redirect_url("not-a-valid-url", result));
    
    cleanup_regex();
    
    return 0;
}
