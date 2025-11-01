
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OUTPUT 4096
#define MAX_COMMAND 256

// Ping function that executes system command
char* ping(const char* url, char* output) {
    char command[MAX_COMMAND];
    FILE* pipe;
    
    #ifdef _WIN32
        snprintf(command, sizeof(command), "ping -n 4 %s", url);
    #else
        snprintf(command, sizeof(command), "ping -c 4 %s", url);
    #endif
    
    pipe = popen(command, "r");
    
    if (!pipe) {
        strcpy(output, "Error: Failed to execute ping command");
        return output;
    }
    
    output[0] = '\\0';
    char buffer[128];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strcat(output, buffer);
    }
    
    pclose(pipe);
    return output;
}

// Simulate request handling
char* handle_request(const char* path, const char* param, char* output) {
    if (strcmp(path, "/ping") == 0) {
        return ping(param, output);
    }
    strcpy(output, "404 Not Found");
    return output;
}

int main() {
    char output[MAX_OUTPUT];
    
    // Test Case 1
    printf("Test Case 1: Ping localhost\\n");
    handle_request("/ping", "localhost", output);
    printf("%s\\n", output);
    printf("\\n==================================================\\n\\n");
    
    // Test Case 2
    printf("Test Case 2: Ping 127.0.0.1\\n");
    handle_request("/ping", "127.0.0.1", output);
    printf("%s\\n", output);
    printf("\\n==================================================\\n\\n");
    
    // Test Case 3
    printf("Test Case 3: Ping google.com\\n");
    handle_request("/ping", "google.com", output);
    printf("%s\\n", output);
    printf("\\n==================================================\\n\\n");
    
    // Test Case 4
    printf("Test Case 4: Ping invalid URL\\n");
    handle_request("/ping", "invalid-url-12345.com", output);
    printf("%s\\n", output);
    printf("\\n==================================================\\n\\n");
    
    // Test Case 5
    printf("Test Case 5: Invalid route\\n");
    handle_request("/invalid", "test", output);
    printf("%s\\n", output);
    
    return 0;
}
