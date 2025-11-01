
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void executeCommand(const char* command) {
    printf("Executing: %s\\n", command);
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Command execution failed with code: %d\\n", result);
    }
}

void executeLsCommand(const char* directory) {
    char command[256];
    snprintf(command, sizeof(command), "ls %s", directory);
    executeCommand(command);
}

int main() {
    // Test case 1: List current directory
    printf("Test Case 1: Current directory\\n");
    executeLsCommand(".");
    
    // Test case 2: List home directory
    printf("\\nTest Case 2: Home directory\\n");
    executeLsCommand("~");
    
    // Test case 3: List with wildcard
    printf("\\nTest Case 3: List with wildcard\\n");
    executeLsCommand("*.c");
    
    // Test case 4: List parent directory
    printf("\\nTest Case 4: Parent directory\\n");
    executeLsCommand("..");
    
    // Test case 5: List /tmp directory
    printf("\\nTest Case 5: /tmp directory\\n");
    executeLsCommand("/tmp");
    
    return 0;
}
