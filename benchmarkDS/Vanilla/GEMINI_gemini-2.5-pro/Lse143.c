#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Using a struct to manage the database for cleaner code
typedef struct {
    char** emails;
    int count;
    int capacity;
} EmailDatabase;

/**
 * Unsubscribes a user by removing their email from the database.
 * This function modifies the database in place.
 *
 * @param db Pointer to the EmailDatabase struct.
 * @param email The email address to unsubscribe.
 * @return A string literal indicating the result of the operation.
 */
const char* unsubscribe(EmailDatabase* db, const char* email) {
    int found_index = -1;
    for (int i = 0; i < db->count; ++i) {
        if (strcmp(db->emails[i], email) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Free the memory for the string we are removing
        free(db->emails[found_index]);

        // Shift elements to the left to fill the gap
        for (int i = found_index; i < db->count - 1; ++i) {
            db->emails[i] = db->emails[i + 1];
        }
        db->count--;
        
        // Return a static success message
        return "User unsubscribed successfully.";
    } else {
        // Return a static not-found message
        return "User is not subscribed.";
    }
}

void printDatabase(const EmailDatabase* db) {
    printf("Current database: { ");
    for (int i = 0; i < db->count; ++i) {
        printf("%s ", db->emails[i]);
    }
    printf("}\n\n");
}

int main() {
    EmailDatabase db;
    db.capacity = 10; // Initial capacity
    db.count = 0;
    db.emails = (char**)malloc(db.capacity * sizeof(char*));
    if (db.emails == NULL) {
        perror("Failed to allocate memory for database");
        return 1;
    }

    // Pre-populate the database
    // strdup allocates memory which we must free later
    db.emails[0] = strdup("test1@example.com");
    db.emails[1] = strdup("user.two@domain.org");
    db.emails[2] = strdup("another@email.net");
    db.count = 3;

    printf("Initial database: { test1@example.com user.two@domain.org another@email.net }\n");
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Unsubscribe an existing user
    const char* email1 = "test1@example.com";
    printf("Attempting to unsubscribe: %s\n", email1);
    printf("Result: %s\n", unsubscribe(&db, email1));
    printDatabase(&db);

    // Test Case 2: Unsubscribe a non-existent user
    const char* email2 = "notfound@example.com";
    printf("Attempting to unsubscribe: %s\n", email2);
    printf("Result: %s\n", unsubscribe(&db, email2));
    printDatabase(&db);

    // Test Case 3: Unsubscribe another existing user
    const char* email3 = "another@email.net";
    printf("Attempting to unsubscribe: %s\n", email3);
    printf("Result: %s\n", unsubscribe(&db, email3));
    printDatabase(&db);

    // Test Case 4: Try to unsubscribe the first user again
    printf("Attempting to unsubscribe: %s\n", email1);
    printf("Result: %s\n", unsubscribe(&db, email1));
    printDatabase(&db);

    // Test Case 5: Unsubscribe the last user
    const char* email5 = "user.two@domain.org";
    printf("Attempting to unsubscribe: %s\n", email5);
    printf("Result: %s\n", unsubscribe(&db, email5));
    printDatabase(&db);

    // Clean up any remaining dynamically allocated strings
    for (int i = 0; i < db.count; ++i) {
        free(db.emails[i]);
    }
    // Free the main array
    free(db.emails);

    return 0;
}