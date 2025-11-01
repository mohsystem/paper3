#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Validates a username to ensure it contains only safe characters.
 * This is crucial to prevent command injection.
 * Allowed characters are: a-z, A-Z, 0-9, underscore, and hyphen.
 *
 * @param username The username string to validate.
 * @return 1 if the username is valid, 0 otherwise.
 */
int isValidUsername(const char* username) {
    if (username == NULL || *username == '\0') {
        return 0;
    }
    size_t len = strlen(username);
    if (len == 0 || len > 32) { // Typical max username length
        return 0;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_' && username[i] != '-') {
            return 0; // Invalid character found
        }
    }
    return 1;
}

/**
 * Executes 'getent passwd <username>' and stores the output in a buffer.
 * It uses popen to safely execute the command and capture its output stream.
 *
 * @param username The username to look up.
 * @param outputBuffer The buffer to store the command's output.
 * @param bufferSize The size of the outputBuffer.
 */
void getUserInfo(const char* username, char* outputBuffer, size_t bufferSize) {
    if (outputBuffer == NULL || bufferSize == 0) {
        return; // No buffer to write to
    }
    outputBuffer[0] = '\0'; // Initialize buffer

    if (!isValidUsername(username)) {
        snprintf(outputBuffer, bufferSize, "Error: Invalid username format.");
        return;
    }

    char command[256];
    // Use snprintf to safely construct the command string and prevent buffer overflows.
    int ret = snprintf(command, sizeof(command), "getent passwd %s", username);
    if (ret < 0 || (size_t)ret >= sizeof(command)) {
        snprintf(outputBuffer, bufferSize, "Error: Failed to construct command string.");
        return;
    }

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        snprintf(outputBuffer, bufferSize, "Error: popen() failed.");
        return;
    }

    // Read the output from the pipe into the buffer
    size_t bytesRead = fread(outputBuffer, 1, bufferSize - 1, pipe);
    outputBuffer[bytesRead] = '\0'; // Null-terminate the string

    pclose(pipe);
}

int main() {
    const char* testUsernames[] = {
        "root",                 // Test Case 1: A valid, existing user
        "nonexistentuser12345", // Test Case 2: A valid but likely non-existing user
        "user name",            // Test Case 3: Invalid format (space)
        "root;whoami",          // Test Case 4: Command injection attempt
        ""                      // Test Case 5: Empty username
    };
    int numTests = sizeof(testUsernames) / sizeof(testUsernames[0]);

    for (int i = 0; i < numTests; ++i) {
        char resultBuffer[512];
        const char* user = testUsernames[i];
        printf("---- Testing for username: '%s' ----\n", user);
        
        getUserInfo(user, resultBuffer, sizeof(resultBuffer));
        
        if (strlen(resultBuffer) == 0) {
             printf("No output received (user may not exist or command failed silently).\n");
        } else {
             printf("%s", resultBuffer);
             // Ensure output ends with a newline
             if (resultBuffer[strlen(resultBuffer) - 1] != '\n') {
                 printf("\n");
             }
        }
        printf("----------------------------------------\n\n");
    }

    return 0;
}