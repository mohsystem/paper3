
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OUTPUT_SIZE 4096
#define MAX_URL_SIZE 256

// Ping function that executes system ping command
char* ping(const char* url, char* output) {
    char command[MAX_URL_SIZE + 50];
    FILE* pipe;
    char buffer[128];
    
    output[0] = '\\0';
    
    #ifdef _WIN32
        snprintf(command, sizeof(command), "ping -n 4 %s", url);
    #else
        snprintf(command, sizeof(command), "ping -c 4 %s", url);
    #endif
    
    pipe = popen(command, "r");
    
    if (pipe == NULL) {
        strcpy(output, "Error: Failed to execute ping command");
        return output;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strlen(output) + strlen(buffer) < MAX_OUTPUT_SIZE - 1) {
            strcat(output, buffer);
        }
    }
    
    pclose(pipe);
    return output;
}

int main() {
    // Test cases
    const char* testUrls[] = {
        "localhost",
        "127.0.0.1",
        "google.com",
        "8.8.8.8",
        "example.com"
    };
    
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);
    char output[MAX_OUTPUT_SIZE];
    
    printf("Testing ping functionality:\\n\\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("=== Pinging: %s ===\\n", testUrls[i]);
        ping(testUrls[i], output);
        printf("%s\\n", output);
        printf("\\n");
    }
    
    return 0;
}
