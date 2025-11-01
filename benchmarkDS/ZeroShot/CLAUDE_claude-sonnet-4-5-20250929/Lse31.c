
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* redirect_url(const char* url) {
    static char result[256];
    char url_copy[512];
    strncpy(url_copy, url, sizeof(url_copy) - 1);
    url_copy[sizeof(url_copy) - 1] = '\\0';
    
    // Simple parsing: find "://" and extract host until next "/"
    char* protocol_end = strstr(url_copy, "://");
    if (protocol_end == NULL) {
        snprintf(result, sizeof(result), "404 Error: Invalid URL");
        return result;
    }
    
    char* host_start = protocol_end + 3;
    char* path_start = strchr(host_start, '/');
    
    char host[256];
    if (path_start != NULL) {
        int host_len = path_start - host_start;
        strncpy(host, host_start, host_len);
        host[host_len] = '\\0';
    } else {
        strncpy(host, host_start, sizeof(host) - 1);
        host[sizeof(host) - 1] = '\\0';
    }
    
    if (strcmp(host, "example.com") == 0) {
        snprintf(result, sizeof(result), "Redirect to: %s", url);
    } else {
        snprintf(result, sizeof(result), "404 Error: Domain not allowed");
    }
    
    return result;
}

int main() {
    // Test cases
    const char* test_cases[] = {
        "https://example.com/page1",
        "http://example.com/login",
        "https://notexample.com/page",
        "https://malicious.com/phishing",
        "https://sub.example.com/resource"
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("URL: %s\\n", test_cases[i]);
        printf("Result: %s\\n\\n", redirect_url(test_cases[i]));
    }
    
    return 0;
}
