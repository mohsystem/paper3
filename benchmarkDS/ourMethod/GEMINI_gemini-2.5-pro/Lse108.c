#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_USERNAME_LEN 32
#define MAX_CMD_LEN (sizeof("getent passwd ") + MAX_USERNAME_LEN)
#define READ_BUFFER_SIZE 256

/**
 * Validates a username. A valid username must be 1-MAX_USERNAME_LEN chars
 * and contain only alphanumeric, dot, underscore, or hyphen characters.
 * @param username The username to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isValidUsername(const char* username) {
    if (username == NULL || username[0] == '\0') {
        return 0;
    }
    size_t len = strlen(username);
    if (len > MAX_USERNAME_LEN) {
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '.' && username[i] != '_' && username[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Executes 'getent passwd <username>' and returns its output.
 * The prompt requires using a function like system(), so this implementation
 * uses popen(), which invokes a shell but allows for output capture.
 * The username is strictly validated to prevent command injection.
 *
 * @param username The username to look up.
 * @return A dynamically allocated string with the command output or an error message.
 *         The caller is responsible for freeing the returned string.
 */
char* getUserInfo(const char* username) {
    // Rule #1: Ensure all input is validated.
    if (!isValidUsername(username)) {
        char* error_msg = malloc(64);
        if (error_msg) {
            snprintf(error_msg, 64, "Error: Username is invalid.");
        }
        return error_msg;
    }

    char command[MAX_CMD_LEN];
    // Rule #2: Use safe string functions.
    snprintf(command, sizeof(command), "getent passwd %s", username);

    // popen() invokes /bin/sh. Security relies on the strict validation above.
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        char* error_msg = malloc(64);
        if (error_msg) {
            snprintf(error_msg, 64, "Error: popen() failed.");
        }
        return error_msg;
    }

    size_t capacity = 512;
    size_t size = 0;
    char* result = malloc(capacity);
    if (!result) {
        pclose(pipe);
        return NULL; // Allocation failure
    }
    result[0] = '\0';

    char buffer[READ_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        if (size + len + 1 > capacity) {
            size_t new_capacity = capacity * 2;
            char* new_result = realloc(result, new_capacity);
            if (!new_result) {
                free(result);
                pclose(pipe);
                return NULL; // Allocation failure
            }
            result = new_result;
            capacity = new_capacity;
        }
        // Rule #3: Ensure buffer operations adhere to boundaries.
        strcat(result, buffer); // Safe due to capacity check
        size += len;
    }

    int status = pclose(pipe);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0 && result[0] == '\0') {
        free(result);
        char* error_msg = malloc(64);
        if (error_msg) {
            snprintf(error_msg, 64, "User not found or an error occurred.");
        }
        return error_msg;
    }
    
    return result;
}

void run_test_case(const char* username) {
    printf("--- Testing for username: '%s' ---\n", username ? username : "NULL");
    char* result = getUserInfo(username);
    if (result) {
        printf("Result:\n%s\n", result);
        free(result);
    } else {
        printf("Result: NULL (Memory allocation failed)\n");
    }
    printf("----------------------------------------\n\n");
}

int main() {
    run_test_case("root");
    run_test_case("nonexistentuser12345");
    run_test_case("user; whoami");
    run_test_case("");
    run_test_case("daemon");
    run_test_case(NULL);
    
    return 0;
}