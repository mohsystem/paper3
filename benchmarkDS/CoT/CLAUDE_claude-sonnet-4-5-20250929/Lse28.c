
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

#define MAX_URL_LENGTH 2048
#define DEFAULT_URL "https://www.example.com/default"

// Global regex pattern compiled once
static regex_t url_pattern;
static int pattern_initialized = 0;

/**
 * Initialize regex pattern once
 */
void init_pattern() {
    if (!pattern_initialized) {
        // Compile regex pattern
        const char* pattern = "^https://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/[a-zA-Z0-9-._~:/?#\\\\[\\\\]@!$&'()*+,;=]*)?$";
        if (regcomp(&url_pattern, pattern, REG_EXTENDED | REG_NOSUB) == 0) {
            pattern_initialized = 1;
        }
    }
}

/**
 * Cleanup regex pattern
 */
void cleanup_pattern() {
    if (pattern_initialized) {
        regfree(&url_pattern);
        pattern_initialized = 0;
    }
}

/**
 * Trim whitespace from string
 */
void trim_string(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && (*start == ' ' || *start == '\\t' || *start == '\\n' || *start == '\\r')) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) {
        *end = '\\0';
        end--;
    }
}

/**
 * Validates and returns redirect URL based on pattern matching
 * @param targetUrl The URL to validate
 * @param result Buffer to store result
 * @param result_size Size of result buffer
 */
void get_redirect_url(const char* targetUrl, char* result, size_t result_size) {
    // Initialize pattern if not done
    init_pattern();
    
    // Null or empty check
    if (targetUrl == NULL || strlen(targetUrl) == 0) {
        strncpy(result, DEFAULT_URL, result_size - 1);
        result[result_size - 1] = '\\0';
        return;
    }
    
    // Copy and trim
    char trimmed[MAX_URL_LENGTH + 1];
    strncpy(trimmed, targetUrl, MAX_URL_LENGTH);
    trimmed[MAX_URL_LENGTH] = '\\0';
    trim_string(trimmed);
    
    // Length validation
    if (strlen(trimmed) > MAX_URL_LENGTH) {
        strncpy(result, DEFAULT_URL, result_size - 1);
        result[result_size - 1] = '\\0';
        return;
    }
    
    // Match against pattern
    if (pattern_initialized && regexec(&url_pattern, trimmed, 0, NULL, 0) == 0) {
        strncpy(result, trimmed, result_size - 1);
        result[result_size - 1] = '\\0';
        return;
    }
    
    // Default fallback
    strncpy(result, DEFAULT_URL, result_size - 1);
    result[result_size - 1] = '\\0';
}

// Test cases
int main() {
    char result[MAX_URL_LENGTH + 1];
    
    get_redirect_url("https://www.example.com/page", result, sizeof(result));
    printf("Test 1: %s\\n", result);
    
    get_redirect_url("https://sub.example.com/resource", result, sizeof(result));
    printf("Test 2: %s\\n", result);
    
    get_redirect_url("https://malicious.com/phishing", result, sizeof(result));
    printf("Test 3: %s\\n", result);
    
    get_redirect_url("", result, sizeof(result));
    printf("Test 4: %s\\n", result);
    
    get_redirect_url("https://example.com/path/to/page?query=value", result, sizeof(result));
    printf("Test 5: %s\\n", result);
    
    cleanup_pattern();
    return 0;
}
