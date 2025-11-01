
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int redirectInvalidUrl(const char* urlString, char* result) {
    if (urlString == NULL || strlen(urlString) == 0) {
        strcpy(result, "Error: URL cannot be empty");
        return 0;
    }
    
    char url[1024];
    strncpy(url, urlString, sizeof(url) - 1);
    url[sizeof(url) - 1] = '\\0';
    
    // Check for protocol
    char* protocolEnd = strstr(url, "://");
    if (protocolEnd == NULL) {
        strcpy(result, "Error: Invalid URL format");
        return 0;
    }
    
    char protocol[10];
    int protocolLen = protocolEnd - url;
    strncpy(protocol, url, protocolLen);
    protocol[protocolLen] = '\\0';
    toLowerCase(protocol);
    
    // Validate protocol
    if (strcmp(protocol, "http") != 0 && strcmp(protocol, "https") != 0) {
        strcpy(result, "Error: Invalid protocol. Only http and https are allowed");
        return 0;
    }
    
    // Extract host
    char* hostStart = protocolEnd + 3;
    char* hostEnd = strchr(hostStart, '/');
    char host[256];
    
    if (hostEnd == NULL) {
        strncpy(host, hostStart, sizeof(host) - 1);
    } else {
        int hostLen = hostEnd - hostStart;
        strncpy(host, hostStart, hostLen);
        host[hostLen] = '\\0';
    }
    
    // Remove port if present
    char* colonPos = strchr(host, ':');
    if (colonPos != NULL) {
        *colonPos = '\\0';
    }
    
    toLowerCase(host);
    
    // Validate domain
    int hostLen = strlen(host);
    int isValid = 0;
    
    if (strcmp(host, "example.com") == 0) {
        isValid = 1;
    } else if (hostLen > 11 && strcmp(host + hostLen - 11, ".example.com") == 0) {
        isValid = 1;
    }
    
    if (!isValid) {
        strcpy(result, "Error: URL must be from example.com domain");
        return 0;
    }
    
    sprintf(result, "Redirect to: %s", urlString);
    return 1;
}

int main() {
    char result[1024];
    
    printf("Test Case 1:\\n");
    redirectInvalidUrl("https://example.com/page", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 2:\\n");
    redirectInvalidUrl("https://sub.example.com/path", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 3:\\n");
    redirectInvalidUrl("https://evil.com", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 4:\\n");
    redirectInvalidUrl("https://example.com.evil.com", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 5:\\n");
    redirectInvalidUrl("ftp://example.com", result);
    printf("%s\\n", result);
    
    return 0;
}
