#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Max number of emails in the database
#define MAX_EMAILS 100
// Max length of an email address
#define MAX_EMAIL_LEN 256

// Helper function to convert a string to lowercase
void toLower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

/**
 * Simulates unsubscribing an email address from a database.
 * The database is represented by an array of strings.
 *
 * @param database The array of subscribed email address strings.
 * @param db_size Pointer to the current number of emails in the database.
 * @param email The email address to unsubscribe.
 * @return A status message indicating success or failure (static string).
 */
const char* unsubscribe(char database[MAX_EMAILS][MAX_EMAIL_LEN], int* db_size, const char* email) {
    // Basic input validation
    if (email == NULL || email[0] == '\0') {
        return "Invalid email address provided.";
    }
    
    // Normalize input email to a temporary lowercase buffer
    char normalized_email[MAX_EMAIL_LEN];
    strncpy(normalized_email, email, MAX_EMAIL_LEN - 1);
    normalized_email[MAX_EMAIL_LEN - 1] = '\0'; // Ensure null termination
    toLower(normalized_email);

    int found_index = -1;

    // Find the email in the database
    for (int i = 0; i < *db_size; i++) {
        // Compare with a lowercase version of the database email
        char db_email_lower[MAX_EMAIL_LEN];
        strncpy(db_email_lower, database[i], MAX_EMAIL_LEN - 1);
        db_email_lower[MAX_EMAIL_LEN - 1] = '\0';
        toLower(db_email_lower);

        if (strcmp(db_email_lower, normalized_email) == 0) {
            found_index = i;
            break;
        }
    }

    // If found, remove it by shifting elements
    if (found_index != -1) {
        for (int i = found_index; i < (*db_size - 1); i++) {
            strcpy(database[i], database[i + 1]);
        }
        (*db_size)--; // Decrease the size of the database
        return "Successfully unsubscribed.";
    } else {
        return "Email address not found in the database.";
    }
}

void print_database(char db[MAX_EMAILS][MAX_EMAIL_LEN], int size) {
    printf("{ ");
    for (int i = 0; i < size; i++) {
        printf("%s ", db[i]);
    }
    printf("}\n");
}

int main() {
    // 1. Initialize the simulated database
    char emailDatabase[MAX_EMAILS][MAX_EMAIL_LEN];
    int db_size = 4;

    strcpy(emailDatabase[0], "user1@example.com");
    strcpy(emailDatabase[1], "user2@example.com");
    strcpy(emailDatabase[2], "subscriber@test.com");
    strcpy(emailDatabase[3], "another.user@domain.org");

    printf("Initial database: ");
    print_database(emailDatabase, db_size);

    // 2. Define test cases
    const char* testEmails[] = {
        "user1@example.com",      // Case 1: Email exists and should be removed.
        "user99@example.com",     // Case 2: Email does not exist.
        "user1@example.com",      // Case 3: Email should now be gone after the first test.
        "SUBSCRIBER@TEST.COM",  // Case 4: Test case-insensitivity.
        ""                        // Case 5: Empty string test.
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    // 3. Run test cases
    for (int i = 0; i < num_tests; i++) {
        printf("\nAttempting to unsubscribe: %s\n", testEmails[i]);
        const char* result = unsubscribe(emailDatabase, &db_size, testEmails[i]);
        printf("Result: %s\n", result);
        printf("Current database: ");
        print_database(emailDatabase, db_size);
    }

    return 0;
}