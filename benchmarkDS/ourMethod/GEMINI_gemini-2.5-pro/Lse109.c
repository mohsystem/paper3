#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PASSWD_FILE "/etc/passwd"

/**
 * Validates the username against a strict whitelist.
 * Allows alphanumeric characters, underscore, dot, and hyphen.
 * This helps prevent command injection and other injection attacks.
 *
 * @param username The string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isValidUsername(const char* username) {
    if (username == NULL || *username == '\0') {
        return 0;
    }
    for (size_t i = 0; username[i] != '\0'; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_' && username[i] != '.' && username[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Finds and returns the line for a given username from /etc/passwd.
 * This implementation reads the file directly to avoid command injection vulnerabilities
 * that would arise from executing external shell commands with user input.
 *
 * NOTE: The returned string is dynamically allocated and must be freed by the caller.
 *
 * @param username The username to search for.
 * @return A dynamically allocated string containing the user's line, or NULL if not found or on error.
 */
char* getUserInfo(const char* username) {
    if (!isValidUsername(username)) {
        fprintf(stderr, "Error: Invalid username format provided.\n");
        return NULL;
    }

    FILE* file = fopen(PASSWD_FILE, "r");
    if (file == NULL) {
        perror("Error opening /etc/passwd");
        return NULL;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char* result = NULL;
    size_t username_len = strlen(username);

    while ((read = getline(&line, &len, file)) != -1) {
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            size_t current_user_len = colon - line;
            if (current_user_len == username_len && strncmp(line, username, username_len) == 0) {
                // Found a match, create a copy to return
                result = strdup(line);
                if (result == NULL) {
                    perror("Failed to allocate memory for result");
                    break;
                }
                // Trim trailing newline if present
                size_t result_len = strlen(result);
                if (result_len > 0 && result[result_len - 1] == '\n') {
                    result[result_len - 1] = '\0';
                }
                break; // Exit loop once found
            }
        }
    }
    
    free(line); // getline allocates memory that needs to be freed
    fclose(file);
    return result;
}

void runTests() {
    const char* testUsernames[] = {
        "root",                 // Test case 1: A common existing user
        "daemon",               // Test case 2: Another common user
        "nonexistentuser12345", // Test case 3: A non-existent user
        "root;whoami",          // Test case 4: A malicious input pattern
        ""                      // Test case 5: An empty input
    };
    int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Searching for user: \"%s\"\n", testUsernames[i]);
        char* result = getUserInfo(testUsernames[i]);
        if (result != NULL) {
            printf("Found: %s\n", result);
            free(result);
        } else {
            printf("Result: Not found or invalid input.\n");
        }
        printf("---------------------------------\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <username>\n", argv[0]);
        printf("\n--- Running Internal Test Cases ---\n");
        runTests();
        return 1;
    }

    char* userInfo = getUserInfo(argv[1]);

    if (userInfo != NULL) {
        printf("%s\n", userInfo);
        free(userInfo);
    } else {
        fprintf(stderr, "User '%s' not found or input was invalid.\n", argv[1]);
    }

    return 0;
}