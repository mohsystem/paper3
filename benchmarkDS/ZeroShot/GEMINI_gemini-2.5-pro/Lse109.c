#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_USERNAME_LENGTH 32

static const char* PASSWD_FILE = "/etc/passwd";

/**
 * Validates if the username contains only allowed characters (alphanumeric, _, -).
 * @param username The username string to validate.
 * @return true if valid, false otherwise.
 */
bool isValidUsername(const char* username) {
    if (username == NULL) {
        return false;
    }
    size_t len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_' && username[i] != '-') {
            return false;
        }
    }
    return true;
}

/**
 * Securely retrieves a user's information from /etc/passwd by parsing the file.
 * This avoids command injection vulnerabilities.
 *
 * @param username The username to search for.
 * @param resultBuffer A buffer to store the result line.
 * @param bufferSize The size of the result buffer.
 */
void getUserInfo(const char* username, char* resultBuffer, size_t bufferSize) {
    if (!isValidUsername(username)) {
        snprintf(resultBuffer, bufferSize, "Error: Invalid username format provided.");
        return;
    }

    FILE* file = fopen(PASSWD_FILE, "r");
    if (file == NULL) {
        snprintf(resultBuffer, bufferSize, "Error: Could not open %s. This program is intended for Unix-like systems.", PASSWD_FILE);
        return;
    }

    char line[MAX_LINE_LENGTH];
    bool found = false;
    while (fgets(line, sizeof(line), file)) {
        // The format of /etc/passwd is `username:password:UID:GID:GECOS:home:shell`
        char lineUsername[MAX_USERNAME_LENGTH + 1] = {0};
        char* firstColon = strchr(line, ':');
        
        if (firstColon != NULL) {
            size_t usernameLen = firstColon - line;
            if (usernameLen <= MAX_USERNAME_LENGTH) {
                strncpy(lineUsername, line, usernameLen);
                lineUsername[usernameLen] = '\0'; // Null-terminate
                
                if (strcmp(lineUsername, username) == 0) {
                    // Remove trailing newline if present
                    size_t lineLen = strlen(line);
                    if (lineLen > 0 && line[lineLen - 1] == '\n') {
                        line[lineLen - 1] = '\0';
                    }
                    snprintf(resultBuffer, bufferSize, "%s", line);
                    found = true;
                    break;
                }
            }
        }
    }

    fclose(file);

    if (!found) {
        snprintf(resultBuffer, bufferSize, "User '%s' not found.", username);
    }
}

int main() {
    const char* testUsers[] = {
        "root",                 // A user that should exist on most systems
        "nobody",               // Another common user
        "nonexistentuser123",   // A user that should not exist
        "daemon",               // A common system user
        "user;rm-rf/"           // An invalid/malicious username attempt
    };
    int numTests = sizeof(testUsers) / sizeof(testUsers[0]);
    char result[MAX_LINE_LENGTH];

    for (int i = 0; i < numTests; ++i) {
        printf("Searching for user: %s\n", testUsers[i]);
        getUserInfo(testUsers[i], result, sizeof(result));
        printf("Result: %s\n", result);
        printf("--------------------\n");
    }

    return 0;
}