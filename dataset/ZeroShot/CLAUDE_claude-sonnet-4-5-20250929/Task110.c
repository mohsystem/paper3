
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_URL_LENGTH 2048

void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

char* trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\\0';
    return str;
}

bool is_valid_protocol(const char* url) {
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0 ||
        strncmp(url, "ftp://", 6) == 0) {
        return true;
    }
    // If no protocol, still valid for domain-only URLs
    if (strstr(url, "://") == NULL) {
        return true;
    }
    return false;
}

bool contains_dangerous_protocol(const char* url) {
    char lower_url[MAX_URL_LENGTH + 1];
    strncpy(lower_url, url, MAX_URL_LENGTH);
    lower_url[MAX_URL_LENGTH] = '\\0';
    to_lower(lower_url);
    
    if (strncmp(lower_url, "javascript:", 11) == 0 ||
        strncmp(lower_url, "data:", 5) == 0 ||
        strncmp(lower_url, "vbscript:", 9) == 0 ||
        strncmp(lower_url, "file:", 5) == 0) {
        return true;
    }
    return false;
}

bool validate_url(const char* url) {
    if (url == NULL || strlen(url) == 0) {
        return false;
    }
    
    char url_copy[MAX_URL_LENGTH + 1];
    strncpy(url_copy, url, MAX_URL_LENGTH);
    url_copy[MAX_URL_LENGTH] = '\\0';
    
    char* trimmed = trim(url_copy);
    
    if (strlen(trimmed) == 0) {
        return false;
    }
    
    // Check maximum length
    if (strlen(trimmed) > MAX_URL_LENGTH) {
        return false;
    }
    
    // Check for whitespace
    if (strchr(trimmed, ' ') != NULL) {
        return false;
    }
    
    // Check for dangerous protocols
    if (contains_dangerous_protocol(trimmed)) {
        return false;
    }
    
    // Basic protocol validation
    if (!is_valid_protocol(trimmed)) {
        return false;
    }
    
    // Check for valid characters (basic validation)
    for (size_t i = 0; i < strlen(trimmed); i++) {
        char c = trimmed[i];
        if (!isalnum((unsigned char)c) && 
            strchr(":/.-_~:?#[]@!$&'()*+,;=%", c) == NULL) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    const char* test_urls[] = {
        "https://www.example.com",
        "http://subdomain.example.com:8080/path?query=value#fragment",
        "ftp://ftp.example.com/file.txt",
        "javascript:alert('XSS')",
        "https://example.com/path with spaces"
    };
    
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);
    
    printf("URL Validation Results:\\n");
    printf("======================\\n");
    
    for (int i = 0; i < num_tests; i++) {
        bool is_valid = validate_url(test_urls[i]);
        printf("URL: %s\\n", test_urls[i]);
        printf("Valid: %s\\n\\n", is_valid ? "true" : "false");
    }
    
    return 0;
}
