
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Security: Maximum lengths to prevent buffer overflows */
#define MAX_COMMAND_LEN 1000
#define MAX_OUTPUT_SIZE (1024 * 1024)  /* 1MB limit */
#define BUFFER_SIZE 128

/* Security: Whitelist of allowed commands */
static const char* ALLOWED_COMMANDS[] = {
    "echo", "date", "pwd", "whoami", "hostname", "uname", "ls", NULL
};

/* Security: Dangerous characters that could enable command injection */
static const char* DANGEROUS_CHARS = ";|&$`<>(){}[]!";

/* Security: Check if character is dangerous */
static bool contains_dangerous_chars(const char* str) {
    if (str == NULL) {
        return true;
    }
    
    /* Security: Check each character against dangerous character list */
    for (size_t i = 0; str[i] != '\\0'; i++) {
        for (size_t j = 0; DANGEROUS_CHARS[j] != '\\0'; j++) {
            if (str[i] == DANGEROUS_CHARS[j]) {
                return true;
            }
        }
    }
    return false;
}

/* Security: Extract base command (first word) from command string */
static bool extract_base_command(const char* command, char* base_cmd, size_t base_cmd_size) {
    if (command == NULL || base_cmd == NULL || base_cmd_size == 0) {
        return false;
    }
    
    /* Security: Initialize buffer */
    memset(base_cmd, 0, base_cmd_size);
    
    /* Security: Find first space or end of string */
    const char* space_pos = strchr(command, ' ');
    size_t len;
    
    if (space_pos != NULL) {
        len = space_pos - command;
    } else {
        len = strlen(command);
    }
    
    /* Security: Check bounds before copy */
    if (len >= base_cmd_size) {
        return false;
    }
    
    /* Security: Safe copy with bounds check */
    strncpy(base_cmd, command, len);
    base_cmd[len] = '\\0';  /* Ensure null termination */
    
    return true;
}

/* Security: Validate command against whitelist */
static bool is_command_allowed(const char* base_cmd) {
    if (base_cmd == NULL) {
        return false;
    }
    
    /* Security: Check against whitelist */
    for (int i = 0; ALLOWED_COMMANDS[i] != NULL; i++) {
        if (strcmp(base_cmd, ALLOWED_COMMANDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

/* Security: Validate command format and content */
static bool validate_command(const char* command) {
    /* Security: Check for NULL pointer */
    if (command == NULL) {
        return false;
    }
    
    /* Security: Check length bounds */
    size_t len = strlen(command);
    if (len == 0 || len > MAX_COMMAND_LEN) {
        return false;
    }
    
    /* Security: Check for dangerous characters */
    if (contains_dangerous_chars(command)) {
        return false;
    }
    
    /* Security: Extract and validate base command */
    char base_cmd[256];
    if (!extract_base_command(command, base_cmd, sizeof(base_cmd))) {
        return false;
    }
    
    /* Security: Check if command is whitelisted */
    return is_command_allowed(base_cmd);
}

/* Security: Execute validated command and return output */
static char* execute_command(const char* command) {
    /* Security: Validate command before execution */
    if (!validate_command(command)) {
        char* error = malloc(100);
        if (error != NULL) {
            snprintf(error, 100, "ERROR: Command not allowed or contains dangerous characters");
        }
        return error;
    }
    
    /* Security: Open pipe in read-only mode */
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        char* error = malloc(100);
        if (error != NULL) {
            snprintf(error, 100, "ERROR: Failed to execute command");
        }
        return error;
    }
    
    /* Security: Allocate initial buffer with size tracking */
    size_t output_size = BUFFER_SIZE;
    size_t output_used = 0;
    char* output = malloc(output_size);
    
    if (output == NULL) {
        pclose(pipe);
        return NULL;
    }
    
    /* Security: Initialize buffer */
    output[0] = '\\0';
    
    char buffer[BUFFER_SIZE];
    
    /* Security: Read output with size limits */
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t buffer_len = strlen(buffer);
        
        /* Security: Check if we need to grow the buffer */
        if (output_used + buffer_len + 1 > output_size) {
            /* Security: Prevent excessive memory allocation */
            if (output_size >= MAX_OUTPUT_SIZE) {
                free(output);
                pclose(pipe);
                char* error = malloc(100);
                if (error != NULL) {
                    snprintf(error, 100, "ERROR: Command output exceeds maximum size limit");
                }
                return error;
            }
            
            /* Security: Grow buffer with bounds check */
            size_t new_size = output_size * 2;
            if (new_size > MAX_OUTPUT_SIZE) {
                new_size = MAX_OUTPUT_SIZE;
            }
            
            char* new_output = realloc(output, new_size);
            if (new_output == NULL) {
                free(output);
                pclose(pipe);
                return NULL;
            }
            
            output = new_output;
            output_size = new_size;
        }
        
        /* Security: Safe string concatenation with bounds check */
        strncat(output, buffer, output_size - output_used - 1);
        output_used += buffer_len;
    }
    
    /* Security: Check command return code */
    int return_code = pclose(pipe);
    
    if (return_code != 0) {
        /* Security: Allocate buffer for error message */
        size_t error_size = output_used + 200;
        char* error = malloc(error_size);
        if (error != NULL) {
            snprintf(error, error_size, "ERROR: Command execution failed with code %d\\n%s", 
                    return_code, output);
        }
        free(output);
        return error;
    }
    
    return output;
}

int main(void) {
    printf("=== Secure Command Executor ===\\n");
    printf("Allowed commands: echo, date, pwd, whoami, hostname, uname, ls\\n");
    printf("Note: Commands with special characters (;|&$`<>(){}[]!) are blocked\\n\\n");
    
    /* Test case 1: Valid echo command */
    printf("Test 1 - echo command:\\n");
    const char* cmd1 = "echo Hello World";
    printf("Input: %s\\n", cmd1);
    char* result1 = execute_command(cmd1);
    if (result1 != NULL) {
        printf("Output: %s\\n", result1);
        free(result1);
    }
    
    /* Test case 2: Valid date command */
    printf("Test 2 - date command:\\n");
    const char* cmd2 = "date";
    printf("Input: %s\\n", cmd2);
    char* result2 = execute_command(cmd2);
    if (result2 != NULL) {
        printf("Output: %s\\n", result2);
        free(result2);
    }
    
    /* Test case 3: Invalid command (not in whitelist) */
    printf("Test 3 - invalid command (not whitelisted):\\n");
    const char* cmd3 = "rm -rf test";
    printf("Input: %s\\n", cmd3);
    char* result3 = execute_command(cmd3);
    if (result3 != NULL) {
        printf("Output: %s\\n", result3);
        free(result3);
    }
    
    /* Test case 4: Command injection attempt with semicolon */
    printf("Test 4 - command injection attempt:\\n");
    const char* cmd4 = "echo test; rm -rf /";
    printf("Input: %s\\n", cmd4);
    char* result4 = execute_command(cmd4);
    if (result4 != NULL) {
        printf("Output: %s\\n", result4);
        free(result4);
    }
    
    /* Test case 5: Command with pipe attempt */
    printf("Test 5 - pipe injection attempt:\\n");
    const char* cmd5 = "echo test | cat";
    printf("Input: %s\\n", cmd5);
    char* result5 = execute_command(cmd5);
    if (result5 != NULL) {
        printf("Output: %s\\n", result5);
        free(result5);
    }
    
    return 0;
}
