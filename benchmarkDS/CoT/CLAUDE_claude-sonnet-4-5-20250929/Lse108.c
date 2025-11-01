
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 32
#define MAX_OUTPUT_LEN 1024

bool is_valid_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN) {
        return false;
    }
    
    // Check for valid characters only: alphanumeric, underscore, hyphen
    for (size_t i = 0; i < strlen(username); i++) {
        char c = username[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

char* get_passwd_info(const char* username, char* output, size_t output_size) {
    // Input validation to prevent command injection
    if (!is_valid_username(username)) {
        snprintf(output, output_size, "Error: Invalid username format");
        return output;
    }
    
    // Safely construct command
    char command[MAX_USERNAME_LEN + 20];
    snprintf(command, sizeof(command), "getent passwd %s", username);
    
    // Execute command and capture output
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(output, output_size, "Error: Failed to execute command");
        return output;
    }
    
    // Read output
    size_t bytes_read = 0;
    char buffer[256];
    output[0] = '\\0';
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && bytes_read < output_size - 1) {
        size_t len = strlen(buffer);
        if (bytes_read + len < output_size - 1) {
            strcat(output, buffer);
            bytes_read += len;
        }
    }
    
    int status = pclose(pipe);
    if (status != 0 && strlen(output) == 0) {
        snprintf(output, output_size, "Error: User not found or command failed");
    }
    
    return output;
}

int main() {
    char output[MAX_OUTPUT_LEN];
    
    // Test cases
    printf("Test 1 - Valid user 'root':\\n");
    printf("%s\\n", get_passwd_info("root", output, sizeof(output)));
    
    printf("\\nTest 2 - Valid user 'nobody':\\n");
    printf("%s\\n", get_passwd_info("nobody", output, sizeof(output)));
    
    printf("\\nTest 3 - Invalid user with special chars '; rm -rf /':\\n");
    printf("%s\\n", get_passwd_info("; rm -rf /", output, sizeof(output)));
    
    printf("\\nTest 4 - Empty username:\\n");
    printf("%s\\n", get_passwd_info("", output, sizeof(output)));
    
    printf("\\nTest 5 - NULL username:\\n");
    printf("%s\\n", get_passwd_info(NULL, output, sizeof(output)));
    
    return 0;
}
