#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // For fork, execvp
#include <sys/wait.h>  // For waitpid
#include <errno.h>     // For errno

/**
 * Executes the 'id <username>' command securely using fork and execvp.
 * This function is safe from command injection because it does not use a shell.
 *
 * @param username The username to look up.
 * @return The exit status of the 'id' command. Returns 0 on success, a positive
 *         integer on command failure (e.g., user not found), and -1 on a
 *         system-level error (e.g., fork failed).
 */
int runIdCommand(const char* username) {
    if (username == NULL || username[0] == '\0') {
        fprintf(stderr, "Error: Username cannot be null or empty.\n");
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // This is the child process
        char* argv[] = {(char*)"id", (char*)username, NULL};

        // execvp will replace the child process with the 'id' command.
        execvp(argv[0], argv);

        // If execvp returns, it means an error occurred.
        perror("execvp failed");
        exit(127); // Standard exit code for command not found or exec failure
    } else {
        // This is the parent process
        int status;
        // Wait for the child process to finish
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }

        // Check if the child process exited normally
        if (WIFEXITED(status)) {
            // Return the exit status of the child process
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "Child process did not terminate normally.\n");
            return -1;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Use the command-line argument if provided
        const char* username = argv[1];
        printf("Running command: id %s\n", username);
        int exitStatus = runIdCommand(username);
        printf("Exit Status: %d\n", exitStatus);
    } else {
        // Run test cases if no argument is given
        printf("No command-line argument provided. Running test cases...\n");
        const char* testUsernames[] = {
            "root",                         // 1. Should succeed (exit 0)
            "nobody",                       // 2. Should succeed (exit 0)
            "non_existent_user_12345",      // 3. Should fail (exit non-zero)
            "root;whoami",                  // 4. Malicious input; should fail
            ""                              // 5. Empty string; should fail
        };
        int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

        for (int i = 0; i < num_tests; ++i) {
            printf("\n------------------------------------\n");
            printf("Testing with username: \"%s\"\n", testUsernames[i]);
            int exitStatus = runIdCommand(testUsernames[i]);
            printf("Exit Status: %d\n", exitStatus);
            printf("------------------------------------\n");
        }
    }
    return 0;
}