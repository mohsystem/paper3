
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_OUTPUT_LENGTH 4096

const char* ALLOWED_COMMANDS[] = {"ls", "pwd", "date", "whoami", "echo", NULL};

int is_allowed_command(const char* command) {
    for (int i = 0; ALLOWED_COMMANDS[i] != NULL; i++) {
        size_t len = strlen(ALLOWED_COMMANDS[i]);
        if (strncmp(command, ALLOWED_COMMANDS[i], len) == 0 &&
            (command[len] == '\\0' || command[len] == ' ')) {
            return 1;
        }
    }
    return 0;
}

int has_dangerous_characters(const char* command) {
    const char* dangerous = ";|&`$(){}<>[]";
    for (int i = 0; command[i] != '\\0'; i++) {
        if (strchr(dangerous, command[i]) != NULL) {
            return 1;
        }
    }
    if (strstr(command, "..") != NULL) {
        return 1;
    }
    return 0;
}

void trim(char* str) {
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\\0';
}

char* execute_command(const char* command) {
    static char result[MAX_OUTPUT_LENGTH];
    char cmd_copy[MAX_COMMAND_LENGTH];
    
    if (command == NULL || strlen(command) == 0) {
        strcpy(result, "Error: Command cannot be empty");
        return result;
    }
    
    strncpy(cmd_copy, command, MAX_COMMAND_LENGTH - 1);
    cmd_copy[MAX_COMMAND_LENGTH - 1] = '\\0';
    trim(cmd_copy);
    
    if (strlen(cmd_copy) == 0) {
        strcpy(result, "Error: Command cannot be empty");
        return result;
    }
    
    // Check for dangerous characters
    if (has_dangerous_characters(cmd_copy)) {
        strcpy(result, "Error: Command contains dangerous characters");
        return result;
    }
    
    // Validate against whitelist
    if (!is_allowed_command(cmd_copy)) {
        strcpy(result, "Error: Command not allowed. Allowed commands: ls, pwd, date, whoami, echo");
        return result;
    }
    
    // Check for path traversal in arguments
    if (strchr(cmd_copy, '/') != NULL || strchr(cmd_copy, '\\\\') != NULL) {\n        char* space = strchr(cmd_copy, ' ');\n        if (space != NULL && (strchr(space + 1, '/') != NULL || strchr(space + 1, '\\\\') != NULL)) {\n            strcpy(result, "Error: Path traversal not allowed in arguments");\n            return result;\n        }\n    }\n    \n    // Execute command\n    char full_command[MAX_COMMAND_LENGTH + 10];\n    snprintf(full_command, sizeof(full_command), "%s 2>&1", cmd_copy);\n    \n    FILE* pipe = popen(full_command, "r");\n    if (pipe == NULL) {\n        strcpy(result, "Error: Failed to execute command");\n        return result;\n    }\n    \n    result[0] = '\\0';\n    char buffer[256];\n    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {\n        strncat(result, buffer, MAX_OUTPUT_LENGTH - strlen(result) - 1);\n    }\n    \n    pclose(pipe);\n    \n    // Remove trailing newline\n    size_t len = strlen(result);\n    if (len > 0 && result[len - 1] == '\
') {\n        result[len - 1] = '\\0';\n    }\n    \n    return result;\n}\n\nint main() {\n    printf("=== Test Case 1: Valid command 'pwd' ===\\n");\n    printf("%s\\n\\n", execute_command("pwd"));\n    \n    printf("=== Test Case 2: Valid command 'date' ===\\n");\n    printf("%s\\n\\n", execute_command("date"));\n    \n    printf("=== Test Case 3: Valid command 'echo' with arguments ===\
");\n    printf("%s\
\
", execute_command("echo Hello World"));\n    \n    printf("=== Test Case 4: Invalid command with dangerous characters ===\
");\n    printf("%s\
\
", execute_command("ls; rm -rf /"));\n    \n    printf("=== Test Case 5: Unauthorized command ===\
");\n    printf("%s\
\
", execute_command("rm testfile.txt"));
    
    return 0;
}
