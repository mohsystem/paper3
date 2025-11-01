#include <stdio.h>
#include <string.h>

#define MAX_EMAILS 10
#define MAX_EMAIL_LEN 50

/**
 * Checks if an email is in the database and removes it if found.
 *
 * @param database A 2D array of chars representing the email database.
 * @param size A pointer to the current number of emails in the database.
 * @param email The email address to unsubscribe.
 * @return A const char* string literal indicating the result of the operation.
 */
const char* unsubscribe(char database[MAX_EMAILS][MAX_EMAIL_LEN], int* size, const char* email) {
    int found_index = -1;
    // Find the email in the database
    for (int i = 0; i < *size; i++) {
        if (strcmp(database[i], email) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Shift elements to the left to "remove" the email
        for (int i = found_index; i < *size - 1; i++) {
            strcpy(database[i], database[i + 1]);
        }
        (*size)--; // Decrease the size of the database
        return "has been unsubscribed successfully.";
    } else {
        return "is not in the database.";
    }
}

// Helper function to print the contents of the database
void print_database(char database[MAX_EMAILS][MAX_EMAIL_LEN], int size) {
    printf("{ ");
    for (int i = 0; i < size; i++) {
        printf("\"%s\" ", database[i]);
    }
    printf("}\n");
}

int main() {
    // Simulate a database with a 2D char array.
    char emailDatabase[MAX_EMAILS][MAX_EMAIL_LEN];
    int db_size = 3;

    // Initialize the database
    strcpy(emailDatabase[0], "user1@example.com");
    strcpy(emailDatabase[1], "user2@example.com");
    strcpy(emailDatabase[2], "user3@example.com");

    printf("Initial database: ");
    print_database(emailDatabase, db_size);

    // --- 5 Test Cases ---
    const char* testEmails[] = {
        "user2@example.com",     // Case 1: Unsubscribe an existing email.
        "user4@example.com",     // Case 2: Try to unsubscribe an email that does not exist.
        "user1@example.com",     // Case 3: Unsubscribe another existing email.
        "user2@example.com",     // Case 4: Try to unsubscribe the first email again.
        "user3@example.com"      // Case 5: Unsubscribe the last remaining email.
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test Case %d ---\n", i + 1);
        const char* currentEmail = testEmails[i];
        printf("Attempting to unsubscribe: %s\n", currentEmail);
        const char* result_msg = unsubscribe(emailDatabase, &db_size, currentEmail);
        printf("Result: %s %s\n", currentEmail, result_msg);
        printf("Current database: ");
        print_database(emailDatabase, db_size);
    }

    return 0;
}