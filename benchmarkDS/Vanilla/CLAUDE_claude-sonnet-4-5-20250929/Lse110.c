
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int runIdCommand(const char* username) {
    char command[256];
    snprintf(command, sizeof(command), "id %s", username);
    
    int exitStatus = system(command);
    
    if (exitStatus == -1) {
        return -1;
    }
    
    // Extract actual exit status from system() return value
    if (WIFEXITED(exitStatus)) {
        return WEXITSTATUS(exitStatus);
    }
    
    return exitStatus;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        const char* username = argv[1];
        int exitStatus = runIdCommand(username);
        printf("Exit status: %d\\n", exitStatus);
    } else {
        // Test cases
        const char* testUsers[] = {"root", "nobody", "daemon", "nonexistentuser123", "bin"};
        
        for (int i = 0; i < 5; i++) {
            printf("\\n=== Testing with user: %s ===\\n", testUsers[i]);
            int exitStatus = runIdCommand(testUsers[i]);
            printf("Exit status: %d\\n", exitStatus);
        }
    }
    
    return 0;
}
