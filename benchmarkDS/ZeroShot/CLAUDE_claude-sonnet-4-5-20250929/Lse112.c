
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int isValidUrl(const char* url) {
    if (url == NULL || strlen(url) == 0 || strlen(url) > 253) {
        return 0;
    }
    
    // Only allow alphanumeric, dots, hyphens, and colons
    for (size_t i = 0; i < strlen(url); i++) {
        char c = url[i];
        if (!isalnum(c) && c != '.' && c != '-' && c != ':') {
            return 0;
        }
    }
    
    return 1;
}

char* ping(const char* url) {
    static char result[4096];
    
    // Validate URL to prevent command injection
    if (!isValidUrl(url)) {
        snprintf(result, sizeof(result), 
                "Invalid URL format. Only alphanumeric characters, dots, hyphens, and colons are allowed.");
        return result;
    }
    
    // Build command safely
    char command[512];
    #ifdef _WIN32
        snprintf(command, sizeof(command), "ping -n 4 %s 2>&1", url);
    #else
        snprintf(command, sizeof(command), "ping -c 4 %s 2>&1", url);
    #endif
    
    // Execute command
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(result, sizeof(result), "Error executing ping command");
        return result;
    }
    
    snprintf(result, sizeof(result), "Ping results for %s:\\n", url);
    size_t offset = strlen(result);
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (offset + strlen(buffer) < sizeof(result) - 1) {
            strcat(result, buffer);
            offset += strlen(buffer);
        }
    }
    
    pclose(pipe);
    return result;
}

int main() {
    // Test cases
    printf("Test 1: Valid domain\\n");
    printf("%s\\n", ping("google.com"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test 2: Valid IP address\\n");
    printf("%s\\n", ping("8.8.8.8"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test 3: Localhost\\n");
    printf("%s\\n", ping("localhost"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test 4: Invalid URL with command injection attempt\\n");
    printf("%s\\n", ping("google.com; ls"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test 5: Invalid URL with special characters\\n");
    printf("%s\\n", ping("google.com && whoami"));
    
    return 0;
}
