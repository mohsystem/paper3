
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

#define MAX_URL_LENGTH 256

int isValidHostname(const char* url) {
    regex_t regex;
    int reti;
    
    // Validate IP or hostname pattern
    const char* hostname_pattern = "^([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$";
    const char* ip_pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
    
    reti = regcomp(&regex, hostname_pattern, REG_EXTENDED);
    if (reti == 0) {
        reti = regexec(&regex, url, 0, NULL, 0);
        regfree(&regex);
        if (reti == 0) return 1;
    }
    
    reti = regcomp(&regex, ip_pattern, REG_EXTENDED);
    if (reti == 0) {
        reti = regexec(&regex, url, 0, NULL, 0);
        regfree(&regex);
        if (reti == 0) return 1;
    }
    
    return 0;
}

void sanitizeUrl(char* url) {
    // Remove protocol
    char* protocol = strstr(url, "://");
    if (protocol) {
        memmove(url, protocol + 3, strlen(protocol + 3) + 1);
    }
    
    // Remove path
    char* path = strchr(url, '/');
    if (path) {
        *path = '\\0';
    }
}

const char* ping(const char* input_url) {
    static char result[512];
    char url[MAX_URL_LENGTH];
    
    // Input validation
    if (!input_url || strlen(input_url) == 0) {
        return "Error: Invalid input - URL cannot be empty";
    }
    
    strncpy(url, input_url, MAX_URL_LENGTH - 1);
    url[MAX_URL_LENGTH - 1] = '\\0';
    
    sanitizeUrl(url);
    
    // Validate format
    if (!isValidHostname(url)) {
        return "Error: Invalid hostname or IP address format";
    }
    
    // Build safe command
    char command[512];
    #ifdef _WIN32
        snprintf(command, sizeof(command), "ping -n 1 -w 5000 %s > nul 2>&1", url);
    #else
        snprintf(command, sizeof(command), "ping -c 1 -W 5 %s > /dev/null 2>&1", url);
    #endif
    
    int ret = system(command);
    
    if (ret == 0) {
        snprintf(result, sizeof(result), "URL %s is alive", url);
    } else {
        snprintf(result, sizeof(result), "URL %s is not alive", url);
    }
    
    return result;
}

int main() {
    // Test cases
    printf("Test 1: %s\\n", ping("8.8.8.8"));
    printf("Test 2: %s\\n", ping("google.com"));
    printf("Test 3: %s\\n", ping("invalid..host"));
    printf("Test 4: %s\\n", ping("; rm -rf /"));
    printf("Test 5: %s\\n", ping(""));
    
    return 0;
}
