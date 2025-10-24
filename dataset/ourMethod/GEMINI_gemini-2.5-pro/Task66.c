#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// POSIX-specific includes for process management
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 64
#define READ_BUFFER_SIZE 256

// A list of allowed commands to prevent execution of arbitrary programs.
const char* ALLOWED_COMMANDS[] = {"ls", "echo", "date", "whoami", NULL};

/**
 * Validates and executes a shell command securely using fork/exec.
 * NOTE: This implementation is for POSIX-compliant systems (Linux, macOS).
 * The caller is responsible for freeing the returned string.
 *
 * @param command The command string to execute.
 * @return A dynamically allocated string with the command's output, or an error message.
 *         Returns NULL on catastrophic allocation failure.
 */
char* executeCommand(const char* command) {
    if (command == NULL || command[0] == '\0') {
        return strdup("Error: Command cannot be null or empty.");
    }
    
    // Disallow shell metacharacters as a primary security measure.
    if (strpbrk(command, ";&|<>`$()") != NULL) {
        return strdup("Error: Command contains forbidden shell metacharacters.");
    }

    // Create a mutable copy for strtok
    char* cmd_copy = strdup(command);
    if (cmd_copy == NULL) {
        return strdup("Error: Memory allocation failed for command copy.");
    }

    char* argv[MAX_ARGS];
    int argc = 0;
    char* token = strtok(cmd_copy, " \t\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;

    if (argc == 0) {
        free(cmd_copy);
        return strdup("Error: Command is empty.");
    }

    // Validate command against the allowlist
    bool is_allowed = false;
    for (int i = 0; ALLOWED_COMMANDS[i] != NULL; ++i) {
        if (strcmp(argv[0], ALLOWED_COMMANDS[i]) == 0) {
            is_allowed = true;
            break;
        }
    }

    if (!is_allowed) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Error: Command '%s' is not allowed.", argv[0]);
        free(cmd_copy);
        return strdup(error_msg);
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        free(cmd_copy);
        return strdup("Error: Failed to create pipe.");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        free(cmd_copy);
        return strdup("Error: Failed to fork process.");
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execvp(argv[0], argv);
        perror("execvp"); // execvp only returns on error
        _exit(127);
    } else { // Parent process
        close(pipefd[1]); // Close write end

        char buffer[READ_BUFFER_SIZE];
        ssize_t count;
        size_t result_size = 1;
        char* result = malloc(result_size);
        if (result == NULL) {
            close(pipefd[0]);
            free(cmd_copy);
            waitpid(pid, NULL, 0);
            return NULL; // Catastrophic failure
        }
        result[0] = '\0';

        while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            char* new_result = realloc(result, result_size + count);
            if (new_result == NULL) {
                free(result);
                close(pipefd[0]);
                free(cmd_copy);
                waitpid(pid, NULL, 0);
                return NULL; // Catastrophic failure
            }
            result = new_result;
            memcpy(result + result_size - 1, buffer, count);
            result_size += count;
            result[result_size - 1] = '\0';
        }
        close(pipefd[0]);
        free(cmd_copy);

        int status;
        waitpid(pid, &status, 0);
        
        char exit_msg[50];
        if (WIFEXITED(status)) {
            snprintf(exit_msg, sizeof(exit_msg), "\nExited with code: %d", WEXITSTATUS(status));
        } else {
            snprintf(exit_msg, sizeof(exit_msg), "\nExited with abnormal termination");
        }
        
        size_t final_size = result_size + strlen(exit_msg);
        char* final_result = realloc(result, final_size);
        if(final_result == NULL){
            free(result);
            return NULL;
        }
        strcat(final_result, exit_msg);
        return final_result;
    }
}

int main() {
    const char* testCommands[] = {
        "ls -l",
        "echo hello from c",
        "grep root /etc/passwd", // Not in allowlist
        "ls; whoami",            // Contains metacharacter
        "",                      // Empty command
        NULL
    };

    for (int i = 0; testCommands[i] != NULL; ++i) {
        printf("Executing command: \"%s\"\n", testCommands[i]);
        char* result = executeCommand(testCommands[i]);
        if (result) {
            printf("Result:\n%s\n", result);
            free(result);
        } else {
            printf("Result: (null) - Memory allocation failed.\n");
        }
        printf("------------------------------------\n");
    }

    return 0;
}