#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/**
 * Executes a bash shell command and returns its output.
 * The caller is responsible for freeing the returned string.
 *
 * @param cmd The shell command to execute.
 * @return A dynamically allocated string with the command's output, or an error message.
 */
char* executeCommand(const char* cmd) {
    // 1. Validation
    if (cmd == NULL || cmd[0] == '\0') {
        return strdup("Validation Error: Command cannot be null or empty.");
    }

    // 2. Append " 2>&1" to redirect stderr to stdout
    size_t cmd_len = strlen(cmd);
    char* cmd_with_redirect = malloc(cmd_len + 6); // For " 2>&1\0"
    if (cmd_with_redirect == NULL) {
        return strdup("Execution Error: malloc failed for command.");
    }
    sprintf(cmd_with_redirect, "%s 2>&1", cmd);

    // 3. Execute command
    FILE* pipe = popen(cmd_with_redirect, "r");
    free(cmd_with_redirect);
    if (!pipe) {
        return strdup("Execution Error: popen() failed!");
    }

    // 4. Capture output
    char buffer[256];
    size_t result_size = 1024;
    size_t current_len = 0;
    char* result = malloc(result_size);
    if (result == NULL) {
        pclose(pipe);
        return strdup("Execution Error: malloc failed for result.");
    }
    result[0] = '\0';

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t buffer_len = strlen(buffer);
        if (current_len + buffer_len + 1 > result_size) {
            result_size *= 2;
            char* new_result = realloc(result, result_size);
            if (new_result == NULL) {
                free(result);
                pclose(pipe);
                return strdup("Execution Error: realloc failed.");
            }
            result = new_result;
        }
        strcat(result, buffer);
        current_len += buffer_len;
    }

    int status = pclose(pipe);

    // 5. Append exit code
    char exit_msg[50];
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    sprintf(exit_msg, "\nExited with code: %d", exit_code);
    
    size_t exit_msg_len = strlen(exit_msg);
    if (current_len + exit_msg_len + 1 > result_size) {
        result_size = current_len + exit_msg_len + 1;
        char* new_result = realloc(result, result_size);
        if (new_result == NULL) {
            free(result);
            return strdup("Execution Error: realloc failed for exit code.");
        }
        result = new_result;
    }
    strcat(result, exit_msg);

    return result;
}

int main() {
    const char* testCommands[] = {
        "echo 'Hello from Bash!'",            // Test Case 1: Simple echo
        "ls -l",                              // Test Case 2: List files
        "ls non_existent_directory",          // Test Case 3: Command with an error
        "echo 'one\\ntwo\\nthree' | grep 'two'", // Test Case 4: Command with a pipe
        "invalidcommand_xyz_123"              // Test Case 5: Invalid command
    };

    for (int i = 0; i < 5; ++i) {
        printf("--- Executing Test Case %d: `%s` ---\n", i + 1, testCommands[i]);
        char* result = executeCommand(testCommands[i]);
        if (result) {
            printf("Result:\n%s\n", result);
            free(result); // Free the memory allocated by executeCommand
        } else {
            printf("Result: NULL (An error occurred)\n");
        }
        printf("--- End Test Case %d ---\n\n", i + 1);
    }

    return 0;
}