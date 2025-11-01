
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <regex.h>

int executeIdCommand(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return -1;
    }
    
    // Sanitize username to prevent command injection
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9_-]+$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\\n");
        return -1;
    }
    
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    if (reti == REG_NOMATCH) {
        fprintf(stderr, "Invalid username format\\n");
        return -1;
    }
    
    char command[256];
    snprintf(command, sizeof(command), "id %s", username);
    
    int exitStatus = system(command);
    
    if (exitStatus == -1) {
        return -1;
    }
    
    return WEXITSTATUS(exitStatus);
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {"root", "nobody", "daemon", "invaliduser123", "admin"};
    int numTests = 5;
    
    if (argc > 1) {
        // Use command line argument if provided
        int exitStatus = executeIdCommand(argv[1]);
        printf("Exit status: %d\\n", exitStatus);
    } else {
        // Run test cases
        printf("Running test cases:\\n\\n");
        for (int i = 0; i < numTests; i++) {
            printf("Testing username: %s\\n", testCases[i]);
            int exitStatus = executeIdCommand(testCases[i]);
            printf("Exit status: %d\\n", exitStatus);
            printf("---\\n");
        }
    }
    
    return 0;
}
