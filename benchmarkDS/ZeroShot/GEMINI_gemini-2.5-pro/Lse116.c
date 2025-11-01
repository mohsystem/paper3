#include <stdio.h>
#include <stdlib.h> // For system()
#include <string.h> // For strlen()
#include <ctype.h>  // For isalnum()

/**
 * Validates a path string to ensure it only contains a whitelist of safe characters.
 * This is a crucial security step to prevent command injection when using system().
 *
 * @param path The path string to validate.
 * @return 1 (true) if the path is secure, 0 (false) otherwise.
 */
int is_path_secure(const char* path) {
    if (path == NULL || path[0] == '\0') {
        return 0; // false for NULL or empty strings
    }
    // A whitelist of allowed characters for a path. This prevents shell
    // metacharacters like ;, &, |, $, ``, (), <, >, etc.
    for (size_t i = 0; i < strlen(path); ++i) {
        char c = path[i];
        if (!isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_') {
            return 0; // false if an invalid character is found
        }
    }
    return 1; // true
}

/**
 * Executes the 'ls' command using system().
 * It uses snprintf to safely format the command string and includes a mandatory
 * security check on the input directory to prevent command injection.
 *
 * @param directory The directory path to list.
 * @return The exit code from system(), or -1 on validation or formatting error.
 */
int executeLsCommand(const char* directory) {
    printf("--- Executing 'ls %s' ---\n", directory ? directory : "NULL");

    // Security Check: Validate the input before using it in any command.
    if (!is_path_secure(directory)) {
        fprintf(stderr, "Error: Invalid or potentially insecure path provided: %s\n", directory ? directory : "NULL");
        printf("------------------------------------\n");
        return -1;
    }

    // Securely construct the command string using snprintf to prevent buffer overflows.
    char command[2048]; // A reasonably large static buffer
    int required_size = snprintf(command, sizeof(command), "ls %s", directory);
    
    if (required_size < 0 || (size_t)required_size >= sizeof(command)) {
        fprintf(stderr, "Error: Failed to construct command string or path is too long.\n");
        printf("------------------------------------\n");
        return -1;
    }

    // Execute the constructed command using system().
    int return_code = system(command);

    printf("Exited with code: %d\n", return_code);
    printf("------------------------------------\n");
    
    return return_code;
}

int main() {
    // Test Case 1: Current directory
    executeLsCommand(".");

    // Test Case 2: A common, valid directory
    executeLsCommand("/tmp");

    // Test Case 3: A validly-formatted but non-existent directory
    executeLsCommand("/non-existent_dir_12345");

    // Test Case 4: A malicious string that should be caught by the validation.
    executeLsCommand("&& whoami");

    // Test Case 5: Another malicious string that should be caught by the validation.
    executeLsCommand(";/bin/bash");

    return 0;
}