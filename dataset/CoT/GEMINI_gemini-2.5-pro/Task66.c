#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Required for POSIX-specific process management
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// 1. Validation: Use an allowlist of safe commands.
const char* ALLOWED_COMMANDS[] = {"ls", "echo", "date", "pwd", "whoami", NULL};

/**
 * Validates, executes a shell command, and returns its output.
 * This implementation is for POSIX-compliant systems (Linux, macOS).
 * It uses fork() and execvp() to avoid shell command injection.
 * NOTE: The caller is responsible for freeing the returned string.
 *
 * @param command The command string to execute.
 * @return A dynamically allocated string with the command's output/error, or NULL on memory error.
 */
char* executeCommand(const char* command) {
    if (command == NULL || strlen(command) == 0 || strspn(command, " \t\n\v\f\r") == strlen(command)) {
        char* error_msg = strdup("Error: Command cannot be null or empty.");
        return error_msg;
    }

    // Create a mutable copy for parsing with strtok
    char* command_copy = strdup(command);
    if (!command_copy) {
        return strdup("Error: Memory allocation failed for command copy.");
    }
    
    // 2. Secure Parsing
    char** argv = NULL;
    int argc = 0;
    char* token = strtok(command_copy, " \t\n");
    while (token != NULL) {
        argc++;
        char** temp = realloc(argv, sizeof(char*) * (argc + 1));
        if (!temp) {
            free(argv);
            free(command_copy);
            return strdup("Error: Memory allocation failed for arguments.");
        }
        argv = temp;
        argv[argc - 1] = token;
        argv[argc] = NULL;
        token = strtok(NULL, " \t\n");
    }

    if (argc == 0) {
        free(argv);
        free(command_copy);
        return strdup("Error: Command is empty after parsing.");
    }

    // 3. Validation against Allowlist
    bool is_allowed = false;
    for (int i = 0; ALLOWED_COMMANDS[i] != NULL; i++) {
        if (strcmp(argv[0], ALLOWED_COMMANDS[i]) == 0) {
            is_allowed = true;
            break;
        }
    }

    if (!is_allowed) {
        size_t msg_len = strlen("Error: Command '' is not allowed.") + strlen(argv[0]) + 1;
        char* error_msg = malloc(msg_len);
        if(error_msg) {
            snprintf(error_msg, msg_len, "Error: Command '%s' is not allowed.", argv[0]);
        }
        free(argv);
        free(command_copy);
        return error_msg ? error_msg : strdup("Error: Unknown command and failed to allocate memory for error message.");
    }
    
    // 4. Secure Execution: fork/exec
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        free(argv);
        free(command_copy);
        return strdup("Error: Failed to create pipe.");
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        free(argv);
        free(command_copy);
        return strdup("Error: Failed to fork process.");
    }

    if (pid == 0) { // Child process
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        execvp(argv[0], argv);
        perror("execvp"); // execvp only returns on error
        _exit(127);
    } else { // Parent process
        close(pipe_fd[1]);

        char* output = NULL;
        size_t output_size = 0;
        char buffer[256];
        ssize_t bytes_read;

        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            char* temp = realloc(output, output_size + bytes_read + 1);
            if (!temp) {
                free(output);
                close(pipe_fd[0]);
                waitpid(pid, NULL, 0); // Clean up child
                free(argv);
                free(command_copy);
                return strdup("Error: Memory allocation failed for output buffer.");
            }
            output = temp;
            memcpy(output + output_size, buffer, bytes_read);
            output_size += bytes_read;
            output[output_size] = '\0';
        }
        close(pipe_fd[0]);

        int status;
        waitpid(pid, &status, 0);
        
        if (output == NULL) { // Handle commands that produce no output
            output = strdup("");
        }

        // Clean up
        free(argv);
        free(command_copy);
        return output;
    }
}

void run_test_case(const char* name, const char* command) {
    printf("--- %s ---\n", name);
    printf("Executing: %s\n", command);
    char* result = executeCommand(command);
    if (result) {
        printf("Result:\n%s\n", result);
        free(result); // Free the dynamically allocated result string
    } else {
        printf("Result: NULL (Memory allocation failed)\n");
    }
}

int main(void) {
    // --- Test Cases ---
    run_test_case("Test Case 1: ls -l", "ls -l");
    run_test_case("Test Case 2: echo Hello World", "echo Hello Secure World");
    run_test_case("Test Case 3: rm -rf /", "rm -rf /");
    run_test_case("Test Case 4: ls; whoami", "ls; whoami");
    run_test_case("Test Case 5: Empty Command", " ");

    return 0;
}