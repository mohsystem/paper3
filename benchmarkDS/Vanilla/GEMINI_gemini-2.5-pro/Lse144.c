#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Simulates a script that unsubscribes an email.
 * @param email The email to unsubscribe.
 * @return 0 for success, non-zero for failure.
 */
int unsubscribeScript(const char* email) {
    printf("-> Calling unsubscribe script for: %s\n", email);
    // Simulate success for emails containing "success", failure otherwise.
    if (strstr(email, "success") != NULL) {
        printf("-> Script returned 0 (success).\n");
        return 0;
    } else {
        printf("-> Script returned 1 (failure).\n");
        return 1;
    }
}

/**
 * @brief Checks if an email exists and attempts to unsubscribe if it does.
 * @param email The email to check.
 * @param database An array of strings representing the database.
 * @param db_size The number of emails in the database.
 * @return 1 if email exists and unsubscribe is successful, 0 otherwise.
 */
int checkAndUnsubscribe(const char* email, const char* database[], int db_size) {
    printf("Checking for email: %s\n", email);
    
    bool found = false;
    for (int i = 0; i < db_size; ++i) {
        if (strcmp(email, database[i]) == 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        printf("Email not found in database.\n");
        return 0; // Email doesn't exist
    }

    printf("Email found in database.\n");
    if (unsubscribeScript(email) == 0) {
        return 1; // Unsubscribe script was successful
    } else {
        return 0; // Unsubscribe script failed
    }
}

int main() {
    // Mock database
    const char* emailDatabase[] = {
        "user.success@example.com",
        "user.fail@example.com",
        "another.success@example.com"
    };
    int db_size = sizeof(emailDatabase) / sizeof(emailDatabase[0]);

    // Test cases
    const char* testEmails[] = {
        "not.in.db@example.com",        // Test Case 1: Email not in DB
        "user.success@example.com",     // Test Case 2: Email in DB, script succeeds
        "user.fail@example.com",        // Test Case 3: Email in DB, script fails
        "another.success@example.com",  // Test Case 4: Another email in DB, script succeeds
        "another.not.in.db@email.com"   // Test Case 5: Another email not in DB
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        int result = checkAndUnsubscribe(testEmails[i], emailDatabase, db_size);
        printf("Final Result: %d\n\n", result);
    }

    return 0;
}