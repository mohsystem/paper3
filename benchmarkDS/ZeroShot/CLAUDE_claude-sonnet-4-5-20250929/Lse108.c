
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 256
#define MAX_OUTPUT_LEN 1024

int isValidUsername(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return 0;
    }
    
    // Check if username contains only alphanumeric, underscore, and hyphen
    for (size_t i = 0; i < strlen(username); i++) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return 0;
        }
    }
    
    return 1;
}

char* executeGetentPasswd(const char* username, char* output, size_t outputSize) {
    // Input validation
    if (!isValidUsername(username)) {
        snprintf(output, outputSize, "Error: Invalid username format");
        return output;
    }
    
    // Build command safely
    char command[MAX_USERNAME_LEN + 20];
    snprintf(command, sizeof(command), "getent passwd %s", username);
    
    // Execute command
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        snprintf(output, outputSize, "Error: Failed to execute command");
        return output;
    }
    
    // Read output
    if (fgets(output, outputSize, fp) != NULL) {
        // Remove trailing newline
        size_t len = strlen(output);
        if (len > 0 && output[len - 1] == '\\n') {
            output[len - 1] = '\\0';
        }
    } else {
        snprintf(output, outputSize, "Error: User not found");
    }
    
    pclose(fp);
    return output;
}

int main() {
    const char* testUsers[] = {"root", "nobody", "daemon", "testuser", "admin"};
    int numTests = 5;
    char output[MAX_OUTPUT_LEN];
    
    printf("Testing getent passwd command:\\n");
    printf("================================\\n\\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("User: %s\\n", testUsers[i]);
        executeGetentPasswd(testUsers[i], output, sizeof(output));
        printf("Result: %s\\n", output);
        printf("--------------------------------\\n\\n");
    }
    
    return 0;
}
