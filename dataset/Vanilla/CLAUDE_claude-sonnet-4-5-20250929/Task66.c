
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int validateCommand(const char* command) {
    // Basic validation - reject potentially dangerous commands
    const char* dangerousPatterns[] = {
        "rm[[:space:]]+-rf[[:space:]]+/",
        "dd[[:space:]]+if=",
        "mkfs",
        "format"
    };
    
    int numPatterns = sizeof(dangerousPatterns) / sizeof(dangerousPatterns[0]);
    
    for (int i = 0; i < numPatterns; i++) {
        regex_t regex;
        int ret = regcomp(&regex, dangerousPatterns[i], REG_EXTENDED);
        
        if (ret == 0) {
            ret = regexec(&regex, command, 0, NULL, 0);
            regfree(&regex);
            
            if (ret == 0) {
                return 0; // Pattern matched - dangerous command
            }
        }
    }
    
    return 1; // Command is safe
}

char* executeCommand(const char* command) {
    static char result[4096];
    result[0] = '\\0';
    
    if (command == NULL || strlen(command) == 0) {
        strcpy(result, "Error: Command cannot be empty");
        return result;
    }
    
    if (!validateCommand(command)) {
        strcpy(result, "Error: Invalid or potentially dangerous command");
        return result;
    }
    
    char fullCommand[1024];
    snprintf(fullCommand, sizeof(fullCommand), "bash -c \\"%s\\" 2>&1", command);
    
    FILE* pipe = popen(fullCommand, "r");
    if (!pipe) {
        strcpy(result, "Error: Failed to execute command");
        return result;
    }
    
    char buffer[256];
    int offset = 0;
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        int len = strlen(buffer);
        if (offset + len < sizeof(result) - 1) {
            strcpy(result + offset, buffer);
            offset += len;
        }
    }
    
    pclose(pipe);
    
    // Remove trailing newline
    int len = strlen(result);
    if (len > 0 && result[len - 1] == '\\n') {
        result[len - 1] = '\\0';
    }
    
    return result;
}

int main() {
    printf("Test Case 1: echo 'Hello World'\\n");
    printf("%s\\n\\n", executeCommand("echo 'Hello World'"));
    
    printf("Test Case 2: ls -la | head -5\\n");
    printf("%s\\n\\n", executeCommand("ls -la | head -5"));
    
    printf("Test Case 3: date\\n");
    printf("%s\\n\\n", executeCommand("date"));
    
    printf("Test Case 4: pwd\\n");
    printf("%s\\n\\n", executeCommand("pwd"));
    
    printf("Test Case 5: echo $((5 + 3))\\n");
    printf("%s\\n\\n", executeCommand("echo $((5 + 3))"));
    
    return 0;
}
