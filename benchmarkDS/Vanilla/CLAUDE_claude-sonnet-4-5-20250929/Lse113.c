
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #define PING_CMD "ping -n 1 "
#else
    #define PING_CMD "ping -c 1 "
#endif

char* ping(const char* url) {
    static char result[4096];
    char command[512];
    char buffer[256];
    char pong[3072] = "";
    
    // Build ping command
    snprintf(command, sizeof(command), "%s%s 2>&1", PING_CMD, url);
    
    // Execute ping command
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        snprintf(result, sizeof(result), "Error executing ping command");
        return result;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strncat(pong, buffer, sizeof(pong) - strlen(pong) - 1);
    }
    
    // Get exit status
    int exit_status = pclose(pipe);
    
    // Format result
    if (exit_status == 0) {
        snprintf(result, sizeof(result), "The URL %s is alive\\n%s", url, pong);
    } else {
        snprintf(result, sizeof(result), "The URL %s is not alive\\n%s", url, pong);
    }
    
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    printf("%s\\n", ping("google.com"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 2:\\n");
    printf("%s\\n", ping("localhost"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 3:\\n");
    printf("%s\\n", ping("8.8.8.8"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 4:\\n");
    printf("%s\\n", ping("invalid-url-that-does-not-exist-12345.com"));
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 5:\\n");
    printf("%s\\n", ping("github.com"));
    
    return 0;
}
