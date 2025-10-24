
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_OUTPUT_LENGTH 10000
#define NUM_ALLOWED_COMMANDS 5

const char* ALLOWED_COMMANDS[] = {"ls", "pwd", "date", "whoami", "echo"};

bool is_allowed_command(const char* cmd) {
    for (int i = 0; i < NUM_ALLOWED_COMMANDS; i++) {
        if (strcmp(cmd, ALLOWED_COMMANDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool contains_forbidden_chars(const char* cmd) {
    const char* forbidden = ";|&><`$\\\\\\n\\r";
    for (size_t i = 0; i < strlen(cmd); i++) {
        if (strchr(forbidden, cmd[i]) != NULL) {
            return true;
        }
    }
    return false;
}

char* execute_command(const char* user_input, char* output, size_t output_size) {
    if (user_input == NULL || strlen(user_input) == 0) {
        snprintf(output, output_size, "Error: Command cannot be empty");
        return output;
    }
    
    // Trim and copy input
    char trimmed[MAX_COMMAND_LENGTH];
    strncpy(trimmed, user_input, MAX_COMMAND_LENGTH - 1);
    trimmed[MAX_COMMAND_LENGTH - 1] = '\\0';
    
    // Remove leading/trailing spaces
    char* start = trimmed;
    while (*start == ' ' || *start == '\\t') start++;
    
    if (strlen(start) == 0) {
        snprintf(output, output_size, "Error: Command cannot be empty");
        return output;
    }
    
    // Validate: no dangerous characters
    if (contains_forbidden_chars(start)) {
        snprintf(output, output_size, "Error: Command contains forbidden characters");
        return output;
    }
    
    // Extract command name
    char command[MAX_COMMAND_LENGTH];
    sscanf(start, "%s", command);
    
    // Whitelist validation
    if (!is_allowed_command(command)) {
        snprintf(output, output_size, "Error: Command '%s' is not in the allowed list", command);
        return output;
    }
    
    // Execute command
    FILE* pipe = popen(start, "r");
    if (pipe == NULL) {
        snprintf(output, output_size, "Error: Failed to execute command");
        return output;
    }
    
    char buffer[128];
    size_t total_read = 0;
    output[0] = '\\0';
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        if (total_read + len < output_size - 1) {
            strcat(output, buffer);
            total_read += len;
        } else {
            strcat(output, "\\n[Output truncated]");
            break;
        }
    }
    
    pclose(pipe);
    
    if (strlen(output) == 0) {
        snprintf(output, output_size, "Command executed successfully (no output)");
    }
    
    return output;
}

int main() {
    printf("=== Secure Command Executor Test Cases ===\\n\\n");
    
    const char* test_cases[] = {
        "ls",
        "pwd",
        "echo Hello World",
        "ls; rm -rf /",
        "cat /etc/passwd"
    };
    
    char output[MAX_OUTPUT_LENGTH];
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d: %s\\n", i + 1, test_cases[i]);
        execute_command(test_cases[i], output, MAX_OUTPUT_LENGTH);
        printf("Result: %s\\n", output);
        printf("---\\n");
    }
    
    return 0;
}
