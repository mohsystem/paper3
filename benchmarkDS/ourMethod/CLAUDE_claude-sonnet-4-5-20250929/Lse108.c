
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/wait.h>

#define MAX_USERNAME_LEN 32
#define MAX_OUTPUT_SIZE 4096

/**
 * Validates username format using regex.
 * Only allows alphanumeric characters, underscore, hyphen, and dot.
 * 
 * @param username The username to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN) {
        return 0;
    }
    
    // Security: Compile regex pattern for username validation
    regex_t regex;
    int ret = regcomp(&regex, "^[a-zA-Z0-9._-]+$", REG_EXTENDED);
    if (ret != 0) {
        return 0;
    }
    
    // Security: Check if username matches pattern
    ret = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0) ? 1 : 0;
}

/**
 * Executes getent passwd command for a given username.
 * Uses popen with validated input to prevent command injection.
 * Validates username format before execution.
 * 
 * @param username The username to query
 * @param output Buffer to store the output (must be at least MAX_OUTPUT_SIZE)
 * @return 0 on success, -1 on error
 */
int get_user_info(const char* username, char* output, size_t output_size) {
    // Security: Initialize output buffer
    if (output == NULL || output_size == 0) {
        return -1;
    }
    memset(output, 0, output_size);
    
    // Security: Validate username format to prevent command injection
    if (!validate_username(username)) {
        snprintf(output, output_size, "Error: Invalid username format");
        return -1;
    }
    
    // Security: Build command with validated input
    char command[256];
    int ret = snprintf(command, sizeof(command), "getent passwd %s", username);
    
    // Security: Check for buffer overflow
    if (ret < 0 || ret >= sizeof(command)) {
        snprintf(output, output_size, "Error: Command too long");
        return -1;
    }
    
    // Security: Use popen in read mode only
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(output, output_size, "Error: Failed to execute command");
        return -1;
    }
    
    // Security: Read with bounds checking
    size_t total_read = 0;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && total_read < output_size - 1) {
        size_t len = strlen(buffer);
        size_t space_left = output_size - total_read - 1;
        size_t to_copy = (len < space_left) ? len : space_left;
        
        // Security: Safe string concatenation with bounds check
        strncat(output, buffer, to_copy);
        total_read += to_copy;
        
        if (total_read >= MAX_OUTPUT_SIZE - 1) {
            break;
        }
    }
    
    // Clean up
    int status = pclose(pipe);
    if (status == -1) {
        snprintf(output, output_size, "Error: Failed to close pipe");
        return -1;
    }
    
    // Check command exit status
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        snprintf(output, output_size, "Error: Command failed with exit code %d", WEXITSTATUS(status));
        return -1;
    }
    
    return 0;
}

int main(void) {
    char output[MAX_OUTPUT_SIZE];
    
    // Test case 1: Valid username
    printf("Test 1 - Valid user 'root':\\n");
    if (get_user_info("root", output, sizeof(output)) == 0) {
        printf("%s\\n", output);
    } else {
        printf("%s\\n", output);
    }
    
    // Test case 2: Valid username
    printf("\\nTest 2 - Valid user 'daemon':\\n");
    if (get_user_info("daemon", output, sizeof(output)) == 0) {
        printf("%s\\n", output);
    } else {
        printf("%s\\n", output);
    }
    
    // Test case 3: Non-existent user
    printf("\\nTest 3 - Non-existent user:\\n");
    if (get_user_info("nonexistentuser12345", output, sizeof(output)) == 0) {
        printf("%s\\n", output);
    } else {
        printf("%s\\n", output);
    }
    
    // Test case 4: Invalid username with shell metacharacter (injection attempt)
    printf("\\nTest 4 - Invalid username (injection attempt):\\n");
    if (get_user_info("root; rm -rf /", output, sizeof(output)) == 0) {
        printf("%s\\n", output);
    } else {
        printf("%s\\n", output);
    }
    
    // Test case 5: Empty username
    printf("\\nTest 5 - Empty username:\\n");
    if (get_user_info("", output, sizeof(output)) == 0) {
        printf("%s\\n", output);
    } else {
        printf("%s\\n", output);
    }
    
    return 0;
}
