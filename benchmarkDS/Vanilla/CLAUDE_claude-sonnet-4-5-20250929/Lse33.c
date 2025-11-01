
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* redirect_invalid_url(const char* url_string) {
    static char result[256];
    char url_copy[256];
    strncpy(url_copy, url_string, sizeof(url_copy) - 1);
    url_copy[sizeof(url_copy) - 1] = '\\0';
    
    // Check if URL starts with http:// or https://
    char* host_start = NULL;
    if (strncmp(url_copy, "http://", 7) == 0) {
        host_start = url_copy + 7;
    } else if (strncmp(url_copy, "https://", 8) == 0) {
        host_start = url_copy + 8;
    } else {
        snprintf(result, sizeof(result), "Error: Invalid URL format.");
        return result;
    }
    
    // Extract host (domain)
    char* path_start = strchr(host_start, '/');
    char host[256];
    if (path_start != NULL) {
        size_t host_len = path_start - host_start;
        strncpy(host, host_start, host_len);
        host[host_len] = '\\0';
    } else {
        strncpy(host, host_start, sizeof(host) - 1);
        host[sizeof(host) - 1] = '\\0';
    }
    
    // Check if domain is example.com
    if (strcmp(host, "example.com") == 0) {
        snprintf(result, sizeof(result), "Redirect to: %s", url_string);
    } else {
        snprintf(result, sizeof(result), "Error: Invalid domain. Only example.com is allowed.");
    }
    
    return result;
}

int main() {
    // Test cases
    const char* test_cases[] = {
        "http://example.com/page1",
        "https://example.com/secure",
        "http://malicious.com/page",
        "http://subdomain.example.com/test",
        "invalid-url-format"
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("Input: %s\\n", test_cases[i]);
        printf("Output: %s\\n", redirect_invalid_url(test_cases[i]));
        printf("\\n");
    }
    
    return 0;
}
