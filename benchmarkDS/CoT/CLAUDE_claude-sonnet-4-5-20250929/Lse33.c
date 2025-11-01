
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    char message[256];
    bool is_valid;
} ValidationResult;

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

ValidationResult redirectInvalidUrl(const char* urlString) {
    ValidationResult result;
    result.is_valid = false;
    
    if (urlString == NULL || strlen(urlString) == 0) {
        strcpy(result.message, "Error: URL parameter is required");
        return result;
    }
    
    // Simple host extraction (looking for :// and then the domain)
    const char* hostStart = strstr(urlString, "://");
    if (hostStart == NULL) {
        strcpy(result.message, "Error: Invalid URL format");
        return result;
    }
    
    hostStart += 3; // Skip "://"
    
    // Find the end of the host (first '/' or end of string)
    char host[256] = {0};
    int i = 0;
    while (hostStart[i] && hostStart[i] != '/' && hostStart[i] != ':' && i < 255) {
        host[i] = hostStart[i];
        i++;
    }
    host[i] = '\\0';
    
    toLowerCase(host);
    
    // Strict domain validation - only allow example.com
    if (strcmp(host, "example.com") == 0 || 
        (strlen(host) > 11 && strcmp(host + strlen(host) - 11, ".example.com") == 0)) {
        strcpy(result.message, urlString);
        result.is_valid = true;
    } else {
        strcpy(result.message, "Error: Invalid domain. Only example.com is allowed");
    }
    
    return result;
}

int main() {
    // Test cases
    ValidationResult r1 = redirectInvalidUrl("https://example.com/page");
    printf("Test 1: %s (Valid: %d)\\n", r1.message, r1.is_valid);
    
    ValidationResult r2 = redirectInvalidUrl("https://www.example.com/page");
    printf("Test 2: %s (Valid: %d)\\n", r2.message, r2.is_valid);
    
    ValidationResult r3 = redirectInvalidUrl("https://evil.com");
    printf("Test 3: %s (Valid: %d)\\n", r3.message, r3.is_valid);
    
    ValidationResult r4 = redirectInvalidUrl("https://example.com.evil.com");
    printf("Test 4: %s (Valid: %d)\\n", r4.message, r4.is_valid);
    
    ValidationResult r5 = redirectInvalidUrl("invalid-url");
    printf("Test 5: %s (Valid: %d)\\n", r5.message, r5.is_valid);
    
    return 0;
}
