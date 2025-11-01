
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

// Validate username format to prevent command injection
bool is_valid_username(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    
    regex_t regex;
    int ret;
    
    // Compile regex pattern: alphanumeric, dash, underscore only
    ret = regcomp(&regex, "^[a-zA-Z0-9_-]+$", REG_EXTENDED);
    if (ret != 0) {
        return false;
    }
    
    ret = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0);
}

int get_user_info(const char* username) {
    // Input validation to prevent command injection
    if (username == NULL || strlen(username) == 0) {
        fprintf(stderr, "Error: Username cannot be empty\\n");
        return -1;
    }
    
    if (!is_valid_username(username)) {
        fprintf(stderr, "Error: Invalid username format\\n");
        return -1;
    }
    
    // Build secure command string with size limit
    char command[256];
    int ret = snprintf(command, sizeof(command), "id %s 2>&1", username);
    
    if (ret < 0 || ret >= sizeof(command)) {
        fprintf(stderr, "Error: Command string too long\\n");
        return -1;
    }
    
    // Execute command and get exit status
    int exitStatus = system(command);
    return WEXITSTATUS(exitStatus);
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* test_users[] = {
        "root", "nobody", "invaliduser123", "test-user", "invalid;user"
    };
    int num_tests = 5;
    
    if (argc > 1) {
        // Use command line argument
        int exit_status = get_user_info(argv[1]);
        printf("Exit status: %d\\n", exit_status);
    } else {
        // Run test cases
        printf("Running test cases:\\n");
        for (int i = 0; i < num_tests; i++) {
            printf("\\nTesting user: %s\\n", test_users[i]);
            int exit_status = get_user_info(test_users[i]);
            printf("Exit status: %d\\n", exit_status);
        }
    }
    
    return 0;
}
