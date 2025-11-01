
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_URL_LENGTH 256
#define MAX_COMMAND_LENGTH 512
#define MAX_OUTPUT_LENGTH 4096

int is_valid_url(const char* url) {
    if (url == NULL || strlen(url) == 0) {
        return 0;
    }
    
    // Create a copy to work with
    char cleanUrl[MAX_URL_LENGTH];
    strncpy(cleanUrl, url, MAX_URL_LENGTH - 1);
    cleanUrl[MAX_URL_LENGTH - 1] = '\\0';
    
    // Remove protocol prefix
    char* start = cleanUrl;
    if (strstr(cleanUrl, "://") != NULL) {
        start = strstr(cleanUrl, "://") + 3;
    }
    
    // Check each character
    for (int i = 0; start[i] != '\\0'; i++) {
        char c = start[i];
        if (!isalnum(c) && c != '.' && c != '-' && c != ':') {
            return 0;
        }
    }
    
    return 1;
}

void extract_hostname(const char* url, char* hostname, size_t size) {
    const char* start = url;
    
    // Skip protocol
    if (strstr(url, "://") != NULL) {
        start = strstr(url, "://") + 3;
    }
    
    // Copy hostname until port or end
    size_t i = 0;
    while (start[i] != '\\0' && start[i] != ':' && i < size - 1) {
        hostname[i] = start[i];
        i++;
    }
    hostname[i] = '\\0';
}

char* ping_url(const char* url) {
    static char result[MAX_OUTPUT_LENGTH];
    
    // Validate URL
    if (!is_valid_url(url)) {
        snprintf(result, MAX_OUTPUT_LENGTH, "Invalid URL format. URL is not alive.");
        return result;
    }
    
    char hostname[MAX_URL_LENGTH];
    extract_hostname(url, hostname, MAX_URL_LENGTH);
    
    char command[MAX_COMMAND_LENGTH];
    
    #ifdef _WIN32
        snprintf(command, MAX_COMMAND_LENGTH, "ping -n 1 %s", hostname);
    #else
        snprintf(command, MAX_COMMAND_LENGTH, "ping -c 1 %s", hostname);
    #endif
    
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        snprintf(result, MAX_OUTPUT_LENGTH, "Error executing ping command for %s", url);
        return result;
    }
    
    char buffer[256];
    char pong[MAX_OUTPUT_LENGTH] = "";
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strncat(pong, buffer, MAX_OUTPUT_LENGTH - strlen(pong) - 1);
    }
    
    int exitCode = pclose(pipe);
    
    if (exitCode == 0) {
        snprintf(result, MAX_OUTPUT_LENGTH, "URL %s is alive.\\n%s", url, pong);
    } else {
        snprintf(result, MAX_OUTPUT_LENGTH, "URL %s is not alive.", url);
    }
    
    return result;
}

int main() {
    const char* testUrls[] = {
        "google.com",
        "127.0.0.1",
        "localhost",
        "example.com",
        "invalid-url-that-does-not-exist-12345.com"
    };
    
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);
    
    printf("=== Ping Test Results ===\\n\\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("Testing: %s\\n", testUrls[i]);
        printf("%s\\n", ping_url(testUrls[i]));
        printf("---\\n");
    }
    
    return 0;
}
