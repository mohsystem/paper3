
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_OUTPUT 4096
#define MAX_COMMAND 256

const char* ALLOWED_COMMANDS[] = {"echo", "date", "whoami", NULL};

int is_command_allowed(const char* command) {
    for (int i = 0; ALLOWED_COMMANDS[i] != NULL; i++) {
        if (strcmp(command, ALLOWED_COMMANDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_argument_safe(const char* argument) {
    if (argument == NULL || strlen(argument) == 0) {
        return 1;
    }
    
    regex_t regex;
    int ret;
    
    ret = regcomp(&regex, "^[a-zA-Z0-9._-]+$", REG_EXTENDED);
    if (ret) {
        return 0;
    }
    
    ret = regexec(&regex, argument, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

char* execute_command(const char* command, const char* argument, char* output, size_t output_size) {
    // Validate command is in whitelist
    if (!is_command_allowed(command)) {
        snprintf(output, output_size, "Error: Command not allowed. Allowed commands: echo, date, whoami");
        return output;
    }
    
    // Validate argument (no special characters for shell injection)
    if (!is_argument_safe(argument)) {
        snprintf(output, output_size, "Error: Invalid argument. Only alphanumeric characters, dots, hyphens, and underscores allowed.");
        return output;
    }
    
    // Build safe command string
    char full_command[MAX_COMMAND];
    if (argument != NULL && strlen(argument) > 0) {
        snprintf(full_command, sizeof(full_command), "%s %s 2>&1", command, argument);
    } else {
        snprintf(full_command, sizeof(full_command), "%s 2>&1", command);
    }
    
    // Execute command safely
    FILE* pipe = popen(full_command, "r");
    if (!pipe) {
        snprintf(output, output_size, "Error: Failed to execute command");
        return output;
    }
    
    size_t pos = 0;
    char buffer[128];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && pos < output_size - 1) {
        size_t len = strlen(buffer);
        if (pos + len < output_size - 1) {
            strcpy(output + pos, buffer);
            pos += len;
        } else {
            break;
        }
    }
    
    output[pos] = '\\0';
    
    // Remove trailing newline
    if (pos > 0 && output[pos - 1] == '\\n') {
        output[pos - 1] = '\\0';
    }
    
    pclose(pipe);
    return output;
}

int main() {
    char output[MAX_OUTPUT];
    
    printf("=== Secure OS Command Execution Demo ===\\n\\n");
    
    // Test Case 1: Valid echo command with safe argument
    printf("Test 1 - Echo command:\\n");
    execute_command("echo", "Hello_World", output, sizeof(output));
    printf("%s\\n\\n", output);
    
    // Test Case 2: Date command (no argument)
    printf("Test 2 - Date command:\\n");
    execute_command("date", "", output, sizeof(output));
    printf("%s\\n\\n", output);
    
    // Test Case 3: Invalid command (not in whitelist)
    printf("Test 3 - Invalid command:\\n");
    execute_command("rm", "file.txt", output, sizeof(output));
    printf("%s\\n\\n", output);
    
    // Test Case 4: Command injection attempt (should be blocked)
    printf("Test 4 - Injection attempt:\\n");
    execute_command("echo", "test; rm -rf /", output, sizeof(output));
    printf("%s\\n\\n", output);
    
    // Test Case 5: Whoami command
    printf("Test 5 - Whoami command:\\n");
    execute_command("whoami", "", output, sizeof(output));
    printf("%s\\n\\n", output);
    
    return 0;
}
