
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_URL_LENGTH 2048
#define MAX_HOST_LENGTH 256

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int isAllowedDomain(const char* host) {
    const char* allowedDomains[] = {"example.com", "www.example.com"};
    int numDomains = 2;
    
    for (int i = 0; i < numDomains; i++) {
        if (strcmp(host, allowedDomains[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int extractHost(const char* url, char* host) {
    const char* start = url;
    
    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
    } else if (strncmp(url, "https://", 8) == 0) {
        start = url + 8;
    }
    
    const char* end = start;
    while (*end && *end != '/' && *end != ':' && *end != '?') {
        end++;
    }
    
    int length = end - start;
    if (length >= MAX_HOST_LENGTH) {
        return 0;
    }
    
    strncpy(host, start, length);
    host[length] = '\\0';
    
    return length > 0;
}

void redirectInvalidUrl(const char* url, char* result) {
    if (url == NULL || strlen(url) == 0) {
        strcpy(result, "Error: URL cannot be empty");
        return;
    }
    
    char host[MAX_HOST_LENGTH];
    
    if (!extractHost(url, host)) {
        strcpy(result, "Error: Invalid URL format");
        return;
    }
    
    toLowerCase(host);
    
    if (isAllowedDomain(host)) {
        snprintf(result, MAX_URL_LENGTH, "Redirect to: %s", url);
    } else {
        strcpy(result, "Error: URL is not from an allowed domain");
    }
}

int main() {
    char result[MAX_URL_LENGTH];
    
    printf("Test 1: ");
    redirectInvalidUrl("https://example.com/page", result);
    printf("%s\\n", result);
    
    printf("Test 2: ");
    redirectInvalidUrl("https://malicious.com/page", result);
    printf("%s\\n", result);
    
    printf("Test 3: ");
    redirectInvalidUrl("https://www.example.com/secure", result);
    printf("%s\\n", result);
    
    printf("Test 4: ");
    redirectInvalidUrl("", result);
    printf("%s\\n", result);
    
    printf("Test 5: ");
    redirectInvalidUrl("https://example.com.malicious.com/fake", result);
    printf("%s\\n", result);
    
    return 0;
}
