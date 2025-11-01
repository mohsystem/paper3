#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_EMAIL_LENGTH 254
#define DB_SIZE 3

// A mock database of email addresses.
const char* emailDatabase[DB_SIZE] = {
    "user1@example.com",
    "success@example.com",
    "fail@example.com"
};

/**
 * @brief Validates an email address format and length.
 * Note: This is a very basic validation, not a full RFC 5322-compliant parser.
 * A production system should use a more robust library for validation.
 * @param email The email string to validate. Must be a null-terminated string.
 * @return true if the email seems valid, false otherwise.
 */
bool isValidEmail(const char* email) {
    if (email == NULL) {
        return false;
    }

    size_t len = strnlen(email, MAX_EMAIL_LENGTH + 1);
    if (len == 0 || len > MAX_EMAIL_LENGTH) {
        return false;
    }

    // Basic check for presence of '@' and '.'
    const char* at_symbol = strchr(email, '@');
    if (at_symbol == NULL || at_symbol == email) { // Must have '@' and not at the start
        return false;
    }

    const char* dot_symbol = strrchr(at_symbol, '.');
    // Must have '.' after '@', not immediately after, and with chars following
    if (dot_symbol == NULL || dot_symbol == at_symbol + 1 || dot_symbol[1] == '\0') {
        return false;
    }

    return true;
}

/**
 * @brief Checks if an email exists in the mock database.
 * @param email The email to check. Must be a null-terminated string.
 * @return true if the email exists, false otherwise.
 */
bool emailExistsInDB(const char* email) {
    if (email == NULL) {
        return false;
    }
    // In a real application, this would query a database.
    for (int i = 0; i < DB_SIZE; ++i) {
        if (strcmp(emailDatabase[i], email) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Simulates calling an external script to unsubscribe an email.
 * @param email The email to unsubscribe. Must be a null-terminated string.
 * @return 0 for success, non-zero for failure.
 */
int callUnsubscribeScript(const char* email) {
    if (email == NULL) return 1;

    printf("Simulating call to unsubscribe script for: %s\n", email);

    // Security Note: In a real-world application, NEVER use system(). It is vulnerable
    // to command injection. On POSIX systems, use fork() and one of the exec() family
    // functions (e.g., execlp) to pass arguments safely in an array.
    // Example (POSIX):
    // #include <unistd.h>
    // #include <sys/wait.h>
    // pid_t pid = fork();
    // if (pid == 0) { // Child process
    //     execlp("/path/to/unsubscribe.sh", "unsubscribe.sh", email, (char*)NULL);
    //     _exit(127); // exec failed, exit child
    // } else if (pid > 0) { // Parent process
    //     int status;
    //     waitpid(pid, &status, 0);
    //     if (WIFEXITED(status)) return WEXITSTATUS(status);
    // }
    // return 1; // Fork failed or other error

    // Mock logic: script fails for a specific email for testing purposes.
    if (strcmp(email, "fail@example.com") == 0) {
        printf("Mock script simulation: FAILED.\n");
        return 1; // Simulate failure
    }

    printf("Mock script simulation: SUCCESS.\n");
    return 0; // Simulate success
}

/**
 * @brief Processes an unsubscribe request based on the specified logic.
 * @param email The email address to process. Must be a null-terminated string.
 * @return 1 if the email exists and is successfully unsubscribed, 0 otherwise.
 */
int processUnsubscribeRequest(const char* email) {
    // 1. Rule#1: Ensure all input is validated.
    if (!isValidEmail(email)) {
        return 0;
    }

    // 2. Check if email exists in the database.
    if (emailExistsInDB(email)) {
        // 3. If it exists, call the unsubscribe script.
        int scriptResult = callUnsubscribeScript(email);
        
        // 4. If the script returns 0, it was a success.
        if (scriptResult == 0) {
            return 1; // Success
        } else {
            return 0; // Script failed
        }
    } else {
        // Email does not exist.
        return 0;
    }
}

int main(void) {
    const char* testEmails[] = {
        "success@example.com",  // Case 1: Exists, script succeeds -> should return 1
        "fail@example.com",     // Case 2: Exists, script fails -> should return 0
        "notfound@example.com", // Case 3: Does not exist -> should return 0
        "invalid-email",        // Case 4: Invalid format -> should return 0
        ""                      // Case 5: Empty string -> should return 0
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Processing test case for: '%s' ---\n", testEmails[i]);
        int result = processUnsubscribeRequest(testEmails[i]);
        printf("Result for '%s': %d\n", testEmails[i], result);
        printf("----------------------------------------\n");
    }

    return 0;
}