#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// This implementation uses POSIX-specific APIs (fork, execvp, pipe).
// It will not compile on non-POSIX systems like Windows without a compatibility layer.
#ifdef __unix__
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#else
// Provide stubs for non-POSIX systems to allow compilation
typedef int pid_t;
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

// 1. Check if a command is in the allowlist.
int is_command_allowed(const char* cmd) {
    const char* allowed_commands[] = {"ls", "pwd", "date", "echo", NULL};
    for (int i = 0; allowed_commands[i] != NULL; ++i) {
        if (strcmp(cmd, allowed_commands[i]) == 0) {
            return 1; // True
        }
    }
    return 0; // False
}

// 2. Validate an argument for forbidden patterns.
int is_argument_valid(const char* arg) {
    if (strstr(arg, "..") != NULL) return 0; // Disallow path traversal
    if (strpbrk(arg, ";&|<>`$()\\{\\}") != NULL) return 0; // Disallow shell metacharacters
    return 1;
}

// NOTE: This function returns a dynamically allocated string that the caller must free.
char* executeSecureCommand(const char* command) {
#ifndef __unix__
    char* error_msg = (char*)malloc(100);
    if(error_msg) snprintf(error_msg, 100, "Error: This implementation is for POSIX-compliant systems only.");
    return error_msg;
#else
    if (command == NULL || strlen(command) == 0) {
        return strdup("Error: Command cannot be null or empty.");
    }
    
    char* cmd_copy = strdup(command);
    if (!cmd_copy) return strdup("Error: Memory allocation failed for command copy.");
    
    char* argv[64]; // Max 63 arguments + NULL terminator
    int argc = 0;
    char* token = strtok(cmd_copy, " \t\n");
    while (token != NULL && argc < 63) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    
    if (argc == 0) {
        free(cmd_copy);
        return strdup("Error: Command is empty after parsing.");
    }
    
    if (!is_command_allowed(argv[0])) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Error: Command '%s' is not allowed.", argv[0]);
        free(cmd_copy);
        return strdup(buffer);
    }
    
    for (int i = 1; i < argc; ++i) {
        if (!is_argument_valid(argv[i])) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Error: Argument '%s' contains forbidden characters or patterns.", argv[i]);
            free(cmd_copy);
            return strdup(buffer);
        }
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) { free(cmd_copy); return strdup("Error: Failed to create pipe."); }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]); close(pipefd[1]);
        free(cmd_copy); return strdup("Error: Failed to fork process.");
    }
    
    if (pid == 0) { // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        // 3. execvp is secure because it does not invoke a shell.
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    } else { // Parent process
        close(pipefd[1]);
        
        size_t capacity = 1024;
        size_t size = 0;
        char* result = (char*)malloc(capacity);
        if (!result) { free(cmd_copy); close(pipefd[0]); return NULL; }
        result[0] = '\0';
        
        int status;
        pid_t wait_result = -1;
        // 4. Timeout mechanism (50 * 100ms = 5 seconds)
        for (int i = 0; i < 50; ++i) {
            wait_result = waitpid(pid, &status, WNOHANG);
            if (wait_result == pid) break;
            usleep(100000);
        }
        
        ssize_t count;
        while ((count = read(pipefd[0], result + size, capacity - size - 1)) > 0) {
            size += count;
            if (size >= capacity - 1) {
                capacity *= 2;
                char* new_result = (char*)realloc(result, capacity);
                if (!new_result) { free(result); free(cmd_copy); close(pipefd[0]); return NULL; }
                result = new_result;
            }
        }
        result[size] = '\0';
        
        close(pipefd[0]);
        free(cmd_copy);
        
        if (wait_result == 0) { // Timeout occurred
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0); // Cleanup zombie
            free(result);
            return strdup("Error: Command timed out.");
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            char error_header[100];
            snprintf(error_header, sizeof(error_header), "Error: Command exited with code %d.\nOutput:\n", WEXITSTATUS(status));
            size_t header_len = strlen(error_header);
            char* full_error = (char*)malloc(header_len + size + 1);
            if (full_error) {
                strcpy(full_error, error_header);
                strcat(full_error, result);
                free(result);
                return full_error;
            }
        }
        
        return result;
    }
#endif
}

int main(void) {
    const char* testCommands[] = {
        "ls -l",                // 1. Valid command
        "echo Hello World",     // 2. Valid command with arguments
        "whoami",               // 3. Invalid command (not in allowlist)
        "ls; rm -rf /",         // 4. Injection attempt ('ls;' rejected)
        "ls ../../",            // 5. Path traversal attempt
        NULL
    };

    for (int i = 0; testCommands[i] != NULL; ++i) {
        printf("Executing: '%s'\n", testCommands[i]);
        printf("-------------------------\n");
        char* result = executeSecureCommand(testCommands[i]);
        if (result) {
            printf("Result:\n%s\n", result);
            free(result); // Free the returned memory
        } else {
            printf("Result: NULL (Memory allocation failed)\n");
        }
        printf("=========================\n");
    }
    return 0;
}