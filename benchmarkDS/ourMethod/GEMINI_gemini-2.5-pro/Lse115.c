#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <regex.h>
#include <errno.h>

// A pattern to validate arguments. It allows alphanumeric characters, dashes, dots, and slashes.
// This is a restrictive whitelist to prevent command injection.
// It does not prevent path traversal (e.g., "ls ../../"), which could be an information disclosure vulnerability.
static const char* SAFE_ARG_PATTERN_C = "^[a-zA-Z0-9_./-]+$";

/**
 * Creates a command string using a format string and arguments.
 * Uses vsnprintf to prevent buffer overflows.
 *
 * @param buffer       The buffer to store the resulting string.
 * @param buffer_size  The size of the buffer.
 * @param format       The format string.
 * @param ...          The arguments to be formatted into the string.
 * @return 0 on success, -1 on failure.
 */
int createCommandString(char* buffer, size_t buffer_size, const char* format, ...) {
    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, buffer_size, format, args);
    va_end(args);

    if (result < 0 || (size_t)result >= buffer_size) {
        fprintf(stderr, "Error: vsnprintf failed or buffer was too small.\n");
        return -1;
    }
    return 0;
}

/**
 * Executes an 'ls' command provided as a single string.
 * The command string is parsed and validated before execution to prevent
 * command injection vulnerabilities. It specifically avoids using a shell.
 *
 * @param command The command string to execute, e.g., "ls -l /tmp".
 */
void executeLsCommand(const char* command) {
    if (command == NULL || *command == '\0') {
        fprintf(stderr, "Error: Command string is null or empty.\n");
        return;
    }

    char* command_copy = strdup(command);
    if (command_copy == NULL) {
        perror("strdup failed");
        return;
    }

    regex_t regex;
    if (regcomp(&regex, SAFE_ARG_PATTERN_C, REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regex.\n");
        free(command_copy);
        return;
    }

    char* argv[256] = {NULL}; // Max 255 arguments, initialized to NULL
    int argc = 0;
    
    char* token = strtok(command_copy, " \t\n");
    while (token != NULL && argc < 255) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    // argv[argc] is already NULL from initialization

    if (argc == 0) {
        fprintf(stderr, "Error: Invalid command format.\n");
        goto cleanup;
    }

    // Security Check 1: The command must be exactly 'ls'.
    if (strcmp(argv[0], "ls") != 0) {
        fprintf(stderr, "Error: Only 'ls' command is allowed. Found: %s\n", argv[0]);
        goto cleanup;
    }

    // Security Check 2: Validate all arguments against a safe pattern.
    for (int i = 1; i < argc; ++i) {
        if (regexec(&regex, argv[i], 0, NULL, 0) != 0) {
            fprintf(stderr, "Error: Argument contains invalid characters: %s\n", argv[i]);
            goto cleanup;
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        goto cleanup;
    }

    if (pid == 0) { // Child process
        // execvp replaces the child process image. It does not use a shell.
        execvp(argv[0], argv);
        perror("execvp failed"); // execvp only returns on error
        _exit(EXIT_FAILURE); // Use _exit in child after fork
    } else { // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        } else {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                 fprintf(stderr, "'ls' command exited with code: %d\n", WEXITSTATUS(status));
            }
        }
    }

cleanup:
    free(command_copy);
    regfree(&regex);
}

int main(void) {
    char buffer[256];

    printf("--- Test Case 1: Simple 'ls .' ---\n");
    if (createCommandString(buffer, sizeof(buffer), "%s %s", "ls", ".") == 0) {
        printf("Executing: %s\n", buffer);
        executeLsCommand(buffer);
    }
    printf("\n");

    printf("--- Test Case 2: 'ls -l' with an option ---\n");
    if (createCommandString(buffer, sizeof(buffer), "%s %s", "ls", "-l") == 0) {
        printf("Executing: %s\n", buffer);
        executeLsCommand(buffer);
    }
    printf("\n");

    printf("--- Test Case 3: 'ls' on a non-existent directory ---\n");
    if (createCommandString(buffer, sizeof(buffer), "%s %s", "ls", "non_existent_dir") == 0) {
        printf("Executing: %s\n", buffer);
        executeLsCommand(buffer);
    }
    printf("\n");

    printf("--- Test Case 4: Malicious command injection attempt (should be blocked) ---\n");
    if (createCommandString(buffer, sizeof(buffer), "%s %s", "ls", ".;whoami") == 0) {
        printf("Executing: %s\n", buffer);
        executeLsCommand(buffer);
    }
    printf("\n");

    printf("--- Test Case 5: Different command attempt (should be blocked) ---\n");
    if (createCommandString(buffer, sizeof(buffer), "%s", "whoami") == 0) {
        printf("Executing: %s\n", buffer);
        executeLsCommand(buffer);
    }
    printf("\n");

    return 0;
}