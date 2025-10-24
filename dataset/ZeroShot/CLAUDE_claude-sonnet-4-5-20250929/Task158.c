
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_OUTPUT 4096
#define MAX_INPUT 256

typedef struct {
    char output[MAX_OUTPUT];
    int success;
} CommandResult;

const char* ALLOWED_COMMANDS[] = {"echo", "date", "pwd", "whoami", "hostname"};
const int NUM_ALLOWED_COMMANDS = 5;

int is_command_allowed(const char* command) {
    for (int i = 0; i < NUM_ALLOWED_COMMANDS; i++) {
        if (strcmp(command, ALLOWED_COMMANDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void sanitize_input(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < output_size - 1; i++) {
        char c = input[i];
        // Remove dangerous characters
        if (c != ';' && c != '&' && c != '|' && c != '`' && 
            c != '$' && c != '(' && c != ')' && c != '<' && 
            c != '>' && c != '\\\\' && c != '\\'' && c != '"') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

CommandResult execute_command(const char* command, const char* argument) {
    CommandResult result;
    result.success = 0;
    memset(result.output, 0, MAX_OUTPUT);
    
    // Validate command against whitelist
    if (!is_command_allowed(command)) {
        snprintf(result.output, MAX_OUTPUT, 
                "Error: Command not allowed. Allowed commands: echo, date, pwd, whoami, hostname");
        return result;
    }
    
    // Sanitize argument
    char sanitized_arg[MAX_INPUT];
    sanitize_input(argument, sanitized_arg, MAX_INPUT);
    
    // Build command string
    char full_command[MAX_INPUT * 2];
    if (strlen(sanitized_arg) > 0) {
        snprintf(full_command, sizeof(full_command), "%s %s 2>&1", command, sanitized_arg);
    } else {
        snprintf(full_command, sizeof(full_command), "%s 2>&1", command);
    }
    
    // Execute command
    FILE* pipe = popen(full_command, "r");
    if (!pipe) {
        snprintf(result.output, MAX_OUTPUT, "Error: Failed to execute command");
        return result;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(result.output + total_read, MAX_OUTPUT - total_read, pipe) != NULL) {
        total_read = strlen(result.output);
        if (total_read >= MAX_OUTPUT - 1) break;
    }
    
    // Remove trailing newline
    size_t len = strlen(result.output);
    if (len > 0 && result.output[len - 1] == '\\n') {
        result.output[len - 1] = '\\0';
    }
    
    pclose(pipe);
    result.success = 1;
    return result;
}

int main() {
    printf("=== Secure OS Command Execution ===\\n\\n");
    
    // Test case 1: Valid echo command
    printf("Test 1 - Echo command:\\n");
    CommandResult r1 = execute_command("echo", "Hello World");
    printf("%s\\n\\n", r1.output);
    
    // Test case 2: Date command
    printf("Test 2 - Date command:\\n");
    CommandResult r2 = execute_command("date", "");
    printf("%s\\n\\n", r2.output);
    
    // Test case 3: Invalid command (blocked)
    printf("Test 3 - Invalid command (should be blocked):\\n");
    CommandResult r3 = execute_command("rm", "-rf /");
    printf("%s\\n\\n", r3.output);
    
    // Test case 4: Command injection attempt (sanitized)
    printf("Test 4 - Command injection attempt:\\n");
    CommandResult r4 = execute_command("echo", "test; rm -rf /");
    printf("%s\\n\\n", r4.output);
    
    // Test case 5: Hostname command
    printf("Test 5 - Hostname command:\\n");
    CommandResult r5 = execute_command("hostname", "");
    printf("%s\\n", r5.output);
    
    return 0;
}
