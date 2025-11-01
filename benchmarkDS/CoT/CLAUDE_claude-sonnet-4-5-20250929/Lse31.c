
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

char* redirect_url(const char* url) {
    static char result[256];
    
    if (url == NULL || strlen(url) == 0) {
        strcpy(result, "404 Error: Invalid URL");
        return result;
    }
    
    // Simple domain extraction from URL
    char url_copy[256];
    strncpy(url_copy, url, sizeof(url_copy) - 1);
    url_copy[sizeof(url_copy) - 1] = '\\0';
    
    char* domain_start = strstr(url_copy, "://");
    if (domain_start) {
        domain_start += 3;
    } else {
        domain_start = url_copy;
    }
    
    char domain[128];
    int i = 0;
    while (domain_start[i] && domain_start[i] != '/' && 
           domain_start[i] != ':' && i < 127) {
        domain[i] = domain_start[i];
        i++;
    }
    domain[i] = '\\0';
    
    to_lowercase(domain);
    
    // Exact domain match - no subdomains allowed
    if (strcmp(domain, "example.com") == 0) {
        snprintf(result, sizeof(result), "Redirect to: %s", url);
    } else {
        strcpy(result, "404 Error: Domain not allowed");
    }
    
    return result;
}

int main() {
    // Test cases
    printf("Test 1: %s\\n", redirect_url("http://example.com/path"));
    printf("Test 2: %s\\n", redirect_url("https://example.com/secure"));
    printf("Test 3: %s\\n", redirect_url("http://evil.com/malicious"));
    printf("Test 4: %s\\n", redirect_url("http://subdomain.example.com/path"));
    printf("Test 5: %s\\n", redirect_url("invalid-url"));
    
    return 0;
}
