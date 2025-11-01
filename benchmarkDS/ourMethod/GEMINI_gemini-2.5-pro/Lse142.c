#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_EMAILS 100
#define MAX_EMAIL_LEN 255 // Includes null terminator

// A simple struct to represent the database
typedef struct {
    char* emails[MAX_EMAILS];
    int count;
} EmailDatabase;

/**
 * @brief Validates an email address for basic format and length.
 * @note This is a basic validation, not a comprehensive regex.
 * @param email The email string to validate.
 * @return true if the email seems valid, false otherwise.
 */
bool isValidEmail(const char* email) {
    if (email == NULL) {
        return false;
    }
    size_t len = strlen(email);
    if (len == 0 || len >= MAX_EMAIL_LEN) {
        return false;
    }

    const char* at_symbol = strchr(email, '@');
    if (at_symbol == NULL || at_symbol == email) { // Must have '@' and not at the start
        return false;
    }
    
    // Must have a '.' after the '@'
    const char* dot = strrchr(at_symbol, '.');
    if (dot == NULL || dot == at_symbol + 1) { // dot exists and isn't right after @
        return false;
    }
    
    return true;
}

/**
 * @brief Unsubscribes an email by removing it from the database.
 * @param db Pointer to the EmailDatabase struct.
 * @param email The email address to unsubscribe.
 * @return A static string indicating the result.
 */
const char* unsubscribe(EmailDatabase* db, const char* email) {
    // Rule #1: Ensure all input is validated.
    if (!isValidEmail(email)) {
        return "Invalid email address format.";
    }

    int found_index = -1;
    for (int i = 0; i < db->count; ++i) {
        if (strcmp(db->emails[i], email) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Free the memory for the email string that is being removed
        free(db->emails[found_index]);

        // Shift all subsequent elements one position to the left
        for (int i = found_index; i < db->count - 1; ++i) {
            db->emails[i] = db->emails[i + 1];
        }
        
        // Decrement the count of emails
        db->count--;
        db->emails[db->count] = NULL; // Null out the last valid pointer

        return "Successfully unsubscribed.";
    } else {
        return "Email address not found in the database.";
    }
}

// Helper function to safely add an email to the database for setup
void addEmail(EmailDatabase* db, const char* email) {
    if (db->count < MAX_EMAILS) {
        // strdup is POSIX, not standard C. Let's do it manually for portability.
        size_t len = strlen(email) + 1;
        if (len > MAX_EMAIL_LEN) {
            fprintf(stderr, "Error: Email too long to add.\n");
            return;
        }
        char* new_email = (char*)malloc(len);
        if (new_email == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return;
        }
        // Use a safe copy function
        #if defined(_WIN32)
            strcpy_s(new_email, len, email);
        #else
            strncpy(new_email, email, len);
            new_email[len-1] = '\0'; // Ensure null termination
        #endif
        
        db->emails[db->count] = new_email;
        db->count++;
    }
}

// Helper function to free all memory allocated by the database
void cleanupDatabase(EmailDatabase* db) {
    for (int i = 0; i < db->count; ++i) {
        free(db->emails[i]);
        db->emails[i] = NULL;
    }
    db->count = 0;
}

int main() {
    EmailDatabase db = { {0}, 0 };

    // Pre-populate the database for demonstration
    addEmail(&db, "test@example.com");
    addEmail(&db, "user1@domain.com");
    addEmail(&db, "another.user@service.net");
    
    printf("--- Test Cases ---\n");

    // Test Case 1: A valid email that exists in the database.
    const char* email1 = "test@example.com";
    printf("Attempting to unsubscribe: %s\n", email1);
    printf("Result: %s\n", unsubscribe(&db, email1));
    printf("Re-checking: %s\n", unsubscribe(&db, email1)); // Should now be not found
    printf("\n");

    // Test Case 2: A valid email that does not exist in the database.
    const char* email2 = "nonexistent@example.com";
    printf("Attempting to unsubscribe: %s\n", email2);
    printf("Result: %s\n", unsubscribe(&db, email2));
    printf("\n");

    // Test Case 3: An invalid email address.
    const char* email3 = "invalid-email";
    printf("Attempting to unsubscribe: %s\n", email3);
    printf("Result: %s\n", unsubscribe(&db, email3));
    printf("\n");

    // Test Case 4: A NULL input.
    const char* email4 = NULL;
    printf("Attempting to unsubscribe: NULL\n");
    printf("Result: %s\n", unsubscribe(&db, email4));
    printf("\n");
    
    // Test Case 5: An empty string input.
    const char* email5 = "";
    printf("Attempting to unsubscribe: \"\"\n");
    printf("Result: %s\n", unsubscribe(&db, email5));
    printf("\n");
    
    // Clean up all allocated memory
    cleanupDatabase(&db);
    
    return 0;
}