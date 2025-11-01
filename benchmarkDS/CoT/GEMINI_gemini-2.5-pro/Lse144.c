#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Simulates a database of subscribed emails for demonstration purposes.
const char* emailDatabase[] = {
    "user.success@example.com",
    "user.fail@example.com",
    "another.success@example.com",
    "test.user@domain.com",
    NULL // Sentinel to mark the end of the array
};

/**
 * @brief Checks if an email exists in the mock database.
 * 
 * @param email The email to search for.
 * @return true if the email is found, false otherwise.
 */
bool emailExists(const char* email) {
    if (!email) return false;
    for (int i = 0; emailDatabase[i] != NULL; ++i) {
        if (strcmp(emailDatabase[i], email) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Simulates an external script that unsubscribes an email.
 * In a real-world scenario, this would be a call to an external service or script.
 * Calling a function is much safer than using system(),
 * which is vulnerable to command injection.
 * 
 * @param email The email to unsubscribe.
 * @return 0 for success, 1 for failure.
 */
int mockUnsubscribeScript(const char* email) {
    // This is a mock implementation.
    // It succeeds if the email contains "success" or "test", and fails otherwise.
    if (email && (strstr(email, "success") != NULL || strstr(email, "test") != NULL)) {
        printf("-> Mock script: Successfully unsubscribed %s\n", email);
        return 0; // Success
    } else {
        printf("-> Mock script: Failed to unsubscribe %s\n", email);
        return 1; // Failure
    }
}

/**
 * @brief Checks if an email exists and attempts to unsubscribe it.
 * 
 * @param email The email address to check and unsubscribe.
 * @return 1 if the email exists and the unsubscribe script succeeds, otherwise 0.
 */
int unsubscribeUser(const char* email) {
    // Input validation: ensure email is not NULL or empty.
    if (email == NULL || email[0] == '\0') {
        printf("Processing '(null or empty)': Invalid input.\n");
        return 0;
    }

    printf("Processing '%s':\n", email);

    // Step 1: Check if the email exists in the database.
    if (emailExists(email)) {
        printf("-> Email found in the database.\n");
        // Step 2: If it exists, call the unsubscribe script.
        int scriptResult = mockUnsubscribeScript(email);

        // Step 3: Check the script's return value.
        if (scriptResult == 0) {
            // Script succeeded, return 1.
            printf("-> Result: 1 (Success)\n");
            return 1;
        } else {
            // Script failed, return 0.
            printf("-> Result: 0 (Script Failed)\n");
            return 0;
        }
    } else {
        // Email does not exist, return 0.
        printf("-> Email not found in the database.\n");
        printf("-> Result: 0\n");
        return 0;
    }
}

int main() {
    const char* testEmails[] = {
        "user.success@example.com",  // Test case 1: Email exists, script succeeds
        "user.fail@example.com",     // Test case 2: Email exists, script fails
        "not.found@example.com",     // Test case 3: Email does not exist
        "test.user@domain.com",      // Test case 4: Another success case
        NULL                         // Test case 5: Invalid null input
    };

    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        int result = unsubscribeUser(testEmails[i]);
        printf("Final return value: %d\n", result);
    }

    return 0;
}