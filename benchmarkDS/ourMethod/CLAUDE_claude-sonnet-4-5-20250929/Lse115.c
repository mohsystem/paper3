
// C implementation
// SECURITY NOTE: This demonstrates the UNSAFE pattern from the prompt.
// system() and popen() with user input create critical vulnerabilities (CWE-78).
// This implementation uses execvp with fork for safe command execution.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_DIR_LEN 255
#define BUFFER_SIZE 256

// Validate directory name: only alphanumeric, hyphens, underscores
int is_valid_directory(const char* directory) {
    if (directory == NULL || *directory == '\\0') {
        return 0;
    }
    
    size_t len = strlen(directory);
    if (len > MAX_DIR_LEN) {
        return 0;
    }
    
    // Check for path traversal
    if (strstr(directory, "..") != NULL || directory[0] == '/') {
        return 0;
    }
    
    // Check each character: only alphanumeric, hyphen, underscore
    for (size_t i = 0; i < len; i++) {
        char c = directory[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_') {
            return 0;
        }
    }
    
    return 1;
}

// Secure command execution with strict validation
char* execute_command(const char* directory) {
    // Input validation
    if (!is_valid_directory(directory)) {
        char* error = malloc(50);
        if (error != NULL) {
            snprintf(error, 50, "Error: Invalid directory parameter");
        }
        return error;
    }
    
    // Create pipe for reading command output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        char* error = malloc(50);
        if (error != NULL) {
            snprintf(error, 50, "Error: Failed to create pipe");
        }
        return error;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        char* error = malloc(50);
        if (error != NULL) {
            snprintf(error, 50, "Error: Fork failed");
        }
        return error;
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close read end
        
        // Redirect stdout to pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(1);
        }
        close(pipefd[1]);
        
        // Execute ls with argument array (NOT shell)
        // This prevents command injection
        char* args[] = {"ls", "-la", (char*)directory, NULL};
        execvp("ls", args);
        
        // If execvp returns, it failed
        _exit(1);
    }
    
    // Parent process
    close(pipefd[1]); // Close write end
    
    // Allocate initial buffer for output
    size_t capacity = 1024;
    size_t size = 0;
    char* output = malloc(capacity);
    if (output == NULL) {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return NULL;
    }
    output[0] = '\\0';
    
    // Read output from pipe
    char buffer[BUFFER_SIZE];
    ssize_t count;
    
    while ((count = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0) {
        // Check if we need to grow the buffer
        if (size + count + 1 > capacity) {
            // Cap at 1MB to prevent excessive memory usage
            if (capacity >= 1048576) {
                break;
            }
            capacity *= 2;
            if (capacity > 1048576) {
                capacity = 1048576;
            }
            char* new_output = realloc(output, capacity);
            if (new_output == NULL) {
                free(output);
                close(pipefd[0]);
                waitpid(pid, NULL, 0);
                return NULL;
            }
            output = new_output;
        }
        
        memcpy(output + size, buffer, count);
        size += count;
        output[size] = '\\0';
    }
    
    close(pipefd[0]);
    
    // Wait for child process
    int status;
    waitpid(pid, &status, 0);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        free(output);
        output = malloc(50);
        if (output != NULL) {
            snprintf(output, 50, "Error: Command execution failed");
        }
        return output;
    }
    
    return output;
}

int main(void) {
    char* result;
    
    printf("Test 1 - Valid directory:\\n");
    result = execute_command("testdir");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test 2 - Valid directory with underscore:\\n");
    result = execute_command("my_folder");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test 3 - Invalid: path traversal attempt:\\n");
    result = execute_command("../etc");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test 4 - Invalid: command injection attempt:\\n");
    result = execute_command("test; rm -rf /");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test 5 - Invalid: pipe attempt:\\n");
    result = execute_command("test | cat /etc/passwd");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
