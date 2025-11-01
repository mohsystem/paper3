#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 32
#define MAX_CMD_LEN 256
#define OUTPUT_BUFFER_SIZE 4096

/**
 * Validates a username against a strict set of rules to prevent command injection.
 * A valid username must start with a letter or underscore, and subsequently
 * contain only letters, numbers, underscores, or hyphens.
 *
 * @param username The username string to validate.
 * @return true if the username is valid, false otherwise.
 */
bool isValidUsername(const char* username) {
    if (username == NULL) {
        return false;
    }

    size_t len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return false;
    }

    // First character must be a letter or underscore
    if (!isalpha((unsigned char)username[0]) && username[0] != '_') {
        return false;
    }

    // Subsequent characters can be letters, numbers, underscore, or hyphen
    for (size_t i = 1; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_' && username[i] != '-') {
            return false;
        }
    }

    return true;
}

/**
 * Executes 'getent passwd <username>' securely and writes the output to a buffer.
 * It performs strict validation on the username before constructing the command.
 *
 * @param username The username to look up.
 * @param outputBuffer A buffer to store the command's output.
 * @param bufferSize The size of the output buffer.
 * @return 0 on success, -1 on failure (e.g., invalid username, command execution error).
 */
int executeGetent(const char* username, char* outputBuffer, size_t bufferSize) {
    if (!isValidUsername(username)) {
        snprintf(outputBuffer, bufferSize, "Error: Invalid username format.\n");
        return -1;
    }

    char command[MAX_CMD_LEN];
    // Use snprintf for safe string formatting to prevent buffer overflows.
    int ret = snprintf(command, sizeof(command), "getent passwd %s", username);
    if (ret < 0 || (size_t)ret >= sizeof(command)) {
        snprintf(outputBuffer, bufferSize, "Error: Failed to construct command.\n");
        return -1;
    }

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        snprintf(outputBuffer, bufferSize, "Error: popen() failed.\n");
        return -1;
    }
    
    // Clear the buffer before reading from the pipe.
    outputBuffer[0] = '\0';
    char* current_pos = outputBuffer;
    size_t remaining_size = bufferSize;
    size_t bytes_read;
    
    // Read the output from the command pipe safely.
    while(remaining_size > 1 && (bytes_read = fread(current_pos, 1, remaining_size - 1, pipe)) > 0){
        current_pos += bytes_read;
        remaining_size -= bytes_read;
    }
    *current_pos = '\0'; // Null-terminate the string.

    int status = pclose(pipe);
    
    // 'getent' exits with a non-zero status if the user is not found.
    if (strlen(outputBuffer) == 0 && status != 0) {
        snprintf(outputBuffer, bufferSize, "User not found or an error occurred.\n");
    }

    return 0;
}


int main() {
    const char* test_usernames[] = {
        "root",                          // Test Case 1: Valid and existing user
        "nobody",                        // Test Case 2: Another valid user
        "thisuserdoesnotexist123",       // Test Case 3: Valid format, but likely non-existent
        "user;whoami",                   // Test Case 4: Command injection attempt (should be rejected)
        "us$er",                         // Test Case 5: Invalid character (should be rejected)
        NULL                             // Sentinel to mark the end of the array
    };

    char output[OUTPUT_BUFFER_SIZE];

    for (int i = 0; test_usernames[i] != NULL; ++i) {
        printf("--- Testing with username: '%s' ---\n", test_usernames[i]);
        executeGetent(test_usernames[i], output, sizeof(output));
        printf("Output:\n%s", output);
        printf("----------------------------------------\n\n");
    }

    return 0;
}