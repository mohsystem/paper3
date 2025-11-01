
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 32

/**
 * Validates that username contains only safe characters.
 * Allows alphanumeric, underscore, and hyphen only.
 * This prevents command injection attacks.
 * 
 * @param username The username to validate
 * @return 1 if valid, 0 if invalid
 */
int isValidUsername(const char* username) {
    if (username == NULL) {
        return 0;
    }

    size_t len = strlen(username);

    // Check length
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return 0;
    }

    // Check each character - whitelist approach
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        // Only allow alphanumeric, underscore, and hyphen
        if (!isalnum((unsigned char)c) && c != '_' && c != '-') {
            return 0;
        }
    }

    return 1;
}

/**
 * Executes the id command for a given username.
 * Returns the exit status of the command.
 * 
 * @param username The username to query (must match safe pattern)
 * @return Exit status: 0 for success, non-zero for failure, -1 for invalid input
 */
int executeIdCommand(const char* username) {
    // Input validation: check for NULL
    if (username == NULL) {
        fprintf(stderr, "Error: Username cannot be NULL\\n");
        return -1;
    }

    // Input validation: sanitize username to prevent command injection
    if (!isValidUsername(username)) {
        fprintf(stderr, "Error: Username is invalid or contains unsafe characters\\n");
        return -1;
    }

    // Use fork and execvp to avoid shell interpretation
    // This prevents command injection attacks
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        fprintf(stderr, "Error: Failed to create process\\n");
        return -1;
    }

    if (pid == 0) {
        // Child process
        // Build argument array - execvp does not use shell
        char* args[3];
        args[0] = "id";
        args[1] = (char*)username;  // Safe after validation
        args[2] = NULL;  // NULL terminator required

        // Execute id command directly without shell
        // execvp searches PATH and does not invoke shell
        execvp("id", args);

        // If execvp returns, it failed
        fprintf(stderr, "Error: Failed to execute command\\n");
        _exit(127);  // Use _exit in child process
    }

    // Parent process
    int status;
    pid_t result = waitpid(pid, &status, 0);

    if (result == -1) {
        fprintf(stderr, "Error: Failed to wait for process\\n");
        return -1;
    }

    // Extract exit status
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        // Process terminated by signal
        fprintf(stderr, "Error: Process terminated by signal\\n");
        return -1;
    }

    return -1;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testUsers[] = {
        "root", "nobody", "testuser123", "user-name", "invalid@user"
    };
    int numTests = 5;

    if (argc > 1) {
        // Use command line argument if provided
        const char* username = argv[1];
        int exitStatus = executeIdCommand(username);
        printf("Exit status: %d\\n", exitStatus);
    } else {
        // Run test cases
        printf("Running test cases:\\n");
        for (int i = 0; i < numTests; i++) {
            printf("\\n--- Testing with username: %s ---\\n", testUsers[i]);
            int exitStatus = executeIdCommand(testUsers[i]);
            printf("Exit status: %d\\n", exitStatus);
        }
    }

    return 0;
}
