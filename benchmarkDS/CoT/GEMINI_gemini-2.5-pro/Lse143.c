#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_EMAILS 100
#define MAX_EMAIL_LEN 100

// Simulating a database of subscribed users with a static array of strings.
static char database[MAX_EMAILS][MAX_EMAIL_LEN];
static int db_size = 0;

/**
 * Unsubscribes a user by removing their email from the database.
 * This function modifies the global database and db_size variables.
 *
 * @param email The email address of the user to unsubscribe.
 * @return A constant string indicating the result of the operation.
 */
const char* unsubscribe(const char* email) {
    // Security: Validate input to prevent processing NULL or empty data.
    if (email == NULL || email[0] == '\0') {
        return "Invalid email address provided.";
    }

    int found_index = -1;
    // Find the email in the database.
    for (int i = 0; i < db_size; ++i) {
        if (strcmp(database[i], email) == 0) {
            found_index = i;
            break;
        }
    }

    // Check if the email was found.
    if (found_index != -1) {
        // If it exists, remove it by shifting subsequent elements left.
        for (int i = found_index; i < db_size - 1; ++i) {
            strcpy(database[i], database[i + 1]);
        }
        // Clear the last valid entry and decrease the size.
        memset(database[db_size - 1], 0, MAX_EMAIL_LEN);
        db_size--;
        return "User has been successfully unsubscribed.";
    } else {
        // If it does not exist, return a message.
        return "User with this email is not subscribed.";
    }
}

void printDatabaseState() {
    printf("Database state: { ");
    for (int i = 0; i < db_size; ++i) {
        printf("\"%s\" ", database[i]);
    }
    printf("}\n");
}

int main() {
    // Pre-populate the simulated database for testing.
    strcpy(database[0], "user1@example.com");
    strcpy(database[1], "user2@example.com");
    strcpy(database[2], "user3@example.com");
    db_size = 3;

    printDatabaseState();
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Unsubscribe an existing user.
    const char* testEmail1 = "user1@example.com";
    printf("Test 1: Unsubscribing '%s'\n", testEmail1);
    const char* result1 = unsubscribe(testEmail1);
    printf("Result: %s\n", result1);
    printDatabaseState();
    printf("\n");

    // Test Case 2: Try to unsubscribe the same user again.
    const char* testEmail2 = "user1@example.com";
    printf("Test 2: Unsubscribing '%s' again\n", testEmail2);
    const char* result2 = unsubscribe(testEmail2);
    printf("Result: %s\n", result2);
    printDatabaseState();
    printf("\n");

    // Test Case 3: Unsubscribe a user that is not in the database.
    const char* testEmail3 = "nonexistent@example.com";
    printf("Test 3: Unsubscribing '%s'\n", testEmail3);
    const char* result3 = unsubscribe(testEmail3);
    printf("Result: %s\n", result3);
    printDatabaseState();
    printf("\n");

    // Test Case 4: Unsubscribe with a NULL email.
    printf("Test 4: Unsubscribing with a NULL email\n");
    const char* result4 = unsubscribe(NULL);
    printf("Result: %s\n", result4);
    printDatabaseState();
    printf("\n");

    // Test Case 5: Unsubscribe with an empty email.
    printf("Test 5: Unsubscribing with an empty email\n");
    const char* result5 = unsubscribe("");
    printf("Result: %s\n", result5);
    printDatabaseState();
    printf("\n");

    return 0;
}