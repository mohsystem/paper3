#include <stdio.h>
#include <stdlib.h> // For system()
#include <string.h>
#include <sys/wait.h> // For WIFEXITED and WEXITSTATUS

/**
 * Runs the 'id <username>' command and returns its exit status.
 * @param username The username to look up.
 * @return The exit status of the 'id' command (0 for success, non-zero for failure).
 */
int runIdCommand(const char* username) {
    // Basic check for invalid input
    if (username == NULL || strlen(username) == 0 || strpbrk(username, " '`;|&") != NULL) {
        return -1;
    }

    char command[256];
    // Safely format the command string, redirecting output to hide it
    int written = snprintf(command, sizeof(command), "id %s > /dev/null 2>&1", username);

    if (written < 0 || written >= sizeof(command)) {
        // Formatting error or buffer too small
        return -1;
    }

    int status = system(command);

    // Check if the command terminated normally and get its exit code
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    // Command did not terminate normally
    return -1;
}

int main() {
    const char* testCases[] = {
        "root",                   // Test case 1: Should exist on most systems (success)
        "daemon",                 // Test case 2: Should also exist on most systems (success)
        "nonexistentuser12345",   // Test case 3: Should not exist (failure)
        "another_fake_user_xyz",  // Test case 4: Should not exist (failure)
        ""                        // Test case 5: Invalid input (failure)
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* user = testCases[i];
        int exitStatus = runIdCommand(user);
        printf("Command 'id %s' exited with status: %d\n", 
               (strlen(user) == 0 ? "<empty>" : user), exitStatus);
    }
    
    return 0;
}