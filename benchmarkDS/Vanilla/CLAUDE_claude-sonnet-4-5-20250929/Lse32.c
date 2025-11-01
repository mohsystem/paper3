
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* redirectInvalidUrl(const char* url) {
    static char result[1024];
    char processedUrl[1024];
    
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        snprintf(processedUrl, sizeof(processedUrl), "http://%s", url);
    } else {
        strncpy(processedUrl, url, sizeof(processedUrl) - 1);
        processedUrl[sizeof(processedUrl) - 1] = '\\0';
    }
    
    char* hostStart = strstr(processedUrl, "://");
    if (hostStart == NULL) {
        strcpy(result, "Error: Invalid URL format");
        return result;
    }
    
    hostStart += 3;
    char* hostEnd = strchr(hostStart, '/');
    char host[256];
    
    if (hostEnd != NULL) {
        size_t hostLen = hostEnd - hostStart;
        strncpy(host, hostStart, hostLen);
        host[hostLen] = '\\0';
    } else {
        strcpy(host, hostStart);
    }
    
    if (strcmp(host, "example.com") == 0 || 
        (strlen(host) > 11 && strcmp(host + strlen(host) - 11, ".example.com") == 0)) {
        strcpy(result, processedUrl);
        return result;
    } else {
        strcpy(result, "Error: Invalid URL. Only example.com URLs are allowed.");
        return result;
    }
}

int main() {
    // Test cases
    printf("Test 1: %s\\n", redirectInvalidUrl("example.com/page"));
    printf("Test 2: %s\\n", redirectInvalidUrl("http://example.com/test"));
    printf("Test 3: %s\\n", redirectInvalidUrl("https://www.example.com/home"));
    printf("Test 4: %s\\n", redirectInvalidUrl("google.com"));
    printf("Test 5: %s\\n", redirectInvalidUrl("http://malicious.com"));
    
    return 0;
}
