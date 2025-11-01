#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>

/**
 * Validates if the username conforms to a safe subset of POSIX rules.
 * Allows letters, digits, underscore, dot, hyphen.
 * Must start with a letter or underscore. Length 1-32.
 * @param username The string to validate.
 * @return 1 if valid, 0 if invalid.
 */
int is_valid_username(const char *username) {
    if (username == NULL) {
        return 0;
    }
    size_t len = strlen(username);
    if (len == 0 || len > 32) {
        return 0;
    }

    if (!isalpha((unsigned char)username[0]) && username[0] != '_') {
        return 0;
    }

    for (size_t i = 1; i < len; ++i) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_' && username[i] != '.' && username[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Executes the 'id' command for a given username in a secure way.
 * This function forks and uses execvp to avoid shell injection.
 *
 * @param username The username to look up.
 * @return The exit code of the 'id' command, or a negative value on error.
 *         -1: Invalid username format.
 *         -2: Fork failed.
 *         -3: waitpid failed.
 *         -4: Child terminated abnormally.
 */
int executeIdCommand(const char* username) {
    if (!is_valid_username(username)) {
        fprintf(stderr, "Error: Invalid username format for '%s'.\n", username);
        return -1; // Indicate invalid input
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork");
        return -2;
    } else if (pid == 0) {
        // Child process
        char *argv[] = {"id", (char *)username, NULL};
        execvp(argv[0], argv);

        // execvp only returns on error
        perror("execvp");
        _exit(127); // Standard exit code for command not found/executable error
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return -3;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Return the child's exit code
        } else {
            // Child terminated abnormally (e.g., by a signal)
            fprintf(stderr, "Error: Child process terminated abnormally for username '%s'.\n", username);
            return -4;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Handle command-line argument if provided
        printf("--- Running with command-line argument ---\n");
        const char *username = argv[1];
        printf("Running command for username: %s\n", username);
        int exitStatus = executeIdCommand(username);
        printf("Exit Status: %d\n", exitStatus);
        printf("----------------------------------------\n\n");
    }
    
    // Run 5 hardcoded test cases
    printf("--- Running 5 Hardcoded Test Cases ---\n");
    const char* testUsernames[] = {
        "root",                 // Test case 1: Valid, existing user
        "daemon",               // Test case 2: Another valid, existing user
        "nonexistentuser12345", // Test case 3: Valid format, but non-existent user
        "invalid;user",         // Test case 4: Invalid format (command injection attempt)
        " a bad user"           // Test case 5: Invalid format (contains spaces)
    };
    int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d: Running command for username '%s'\n", i + 1, testUsernames[i]);
        int exitStatus = executeIdCommand(testUsernames[i]);
        printf("Exit Status: %d\n", exitStatus);
        printf("----------------------------------------\n");
    }

    return 0;
}