
// C - Secure alternative with strict validation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PATH 256
#define MAX_OUTPUT 4096

// SECURE: Validates input before executing command
char* execute_list_command(const char* directory, char* output, size_t output_size) {
    if (directory == NULL || output == NULL || output_size == 0) {
        snprintf(output, output_size, "Error: Invalid parameters\\n");
        return output;
    }
    
    // Validate directory path - prevent injection attacks
    if (strstr(directory, "..") != NULL ||
        strstr(directory, ";") != NULL ||
        strstr(directory, "|") != NULL ||
        strstr(directory, "&") != NULL ||
        strstr(directory, "`") != NULL ||
        strstr(directory, "$") != NULL) {
        snprintf(output, output_size, "Error: Invalid directory path\\n");
        return output;
    }
    
    // Additional character validation
    for (size_t i = 0; i < strlen(directory); i++) {
        char c = directory[i];
        if (!isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_' && c != ' ') {
            snprintf(output, output_size, "Error: Invalid characters in path\\n");
            return output;
        }
    }
    
    // Build safe command with quoted path
    char command[MAX_PATH + 20];
    snprintf(command, sizeof(command), "ls -l \\"%s\\" 2>&1", directory);
    
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(output, output_size, "Error: Failed to execute command\\n");
        return output;
    }
    
    size_t offset = 0;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && offset < output_size - 1) {
        size_t len = strlen(buffer);
        if (offset + len < output_size) {
            strcpy(output + offset, buffer);
            offset += len;
        }
    }
    output[offset] = '\\0';
    
    pclose(pipe);
    return output;
}

int main() {
    char output[MAX_OUTPUT];
    
    printf("Test Case 1: Current directory\\n");
    execute_list_command(".", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("Test Case 2: Root directory\\n");
    execute_list_command("/", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("Test Case 3: Tmp directory\\n");
    execute_list_command("/tmp", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("Test Case 4: Path traversal attempt\\n");
    execute_list_command("../../../etc", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("Test Case 5: Command injection attempt\\n");
    execute_list_command("; rm -rf /", output, sizeof(output));
    printf("%s\\n", output);
    
    return 0;
}
