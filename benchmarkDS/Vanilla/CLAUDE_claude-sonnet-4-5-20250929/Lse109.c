
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void executeCommand(const char* username) {
    char command[256];
    snprintf(command, sizeof(command), "cat /etc/passwd | grep %s", username);
    
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        printf("Error executing command\\n");
        return;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    
    pclose(pipe);
}

int main(int argc, char* argv[]) {
    const char* testUsers[] = {"root", "daemon", "bin", "sys", "nobody"};
    int numTests = 5;
    
    if (argc > 1) {
        printf("User information for: %s\\n", argv[1]);
        executeCommand(argv[1]);
    } else {
        printf("Running test cases:\\n");
        for (int i = 0; i < numTests; i++) {
            printf("=== User: %s ===\\n", testUsers[i]);
            executeCommand(testUsers[i]);
        }
    }
    
    return 0;
}
