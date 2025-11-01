#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DB_SIZE 10
#define MSG_BUFFER_SIZE 512

/**
 * A basic validation for email format.
 * NOTE: This is a simplified check. For production code, a robust library
 * for regex validation is strongly recommended to handle all edge cases.
 * @param email The email string to validate.
 * @return true if the format seems valid, false otherwise.
 */
bool is_valid_email_format(const char* email) {
    if (email == NULL) return false;
    
    const char* at_symbol = strchr(email, '@');
    // Must have exactly one '@' and it cannot be the first character.
    if (at_symbol == NULL || at_symbol == email || strchr(at_symbol + 1, '@') != NULL) {
        return false;
    }
    
    const char* dot_after_at = strchr(at_symbol + 1, '.');
    // Must have a '.' after '@', not immediately after, and not as the last char.
    if (dot_after_at == NULL || dot_after_at == at_symbol + 1 || *(dot_after_at + 1) == '\0') {
        return false;
    }

    return true;
}

/**
 * Unsubscribes an email from a simulated database (array of strings).
 * @param email The email address to unsubscribe.
 * @param database The array of C-strings representing the database.
 * @param db_size Pointer to the current size of the database (will be modified).
 * @param result_msg A buffer to store the output message.
 * @param msg_size The size of the result_msg buffer.
 */
void unsubscribe(const char* email, char* database[], int* db_size, char* result_msg, size_t msg_size) {
    // 1. Input Validation: Check for NULL or empty string.
    if (email == NULL || email[0] == '\0') {
        snprintf(result_msg, msg_size, "Error: Email address cannot be empty.");
        return;
    }
    
    // 2. Input Validation: Check for a valid email format.
    if (!is_valid_email_format(email)) {
        snprintf(result_msg, msg_size, "Error: Invalid email address format.");
        return;
    }
    
    // SECURITY NOTE: In a real C application, use a database library (like libpq for
    // PostgreSQL) that supports parameterized queries to prevent SQL injection.

    // 3. Business Logic: Find the email in the database array.
    int found_index = -1;
    for (int i = 0; i < *db_size; i++) {
        if (database[i] != NULL && strcmp(database[i], email) == 0) {
            found_index = i;
            break;
        }
    }
    
    if (found_index != -1) {
        // Email found: remove it by shifting all subsequent elements left by one.
        for (int i = found_index; i < (*db_size - 1); i++) {
            database[i] = database[i + 1];
        }
        database[*db_size - 1] = NULL; // Clear the now-duplicate last pointer.
        (*db_size)--;
        snprintf(result_msg, msg_size, "Success: %s has been unsubscribed.", email);
    } else {
        snprintf(result_msg, msg_size, "Info: %s was not found in our database.", email);
    }
}

// Helper function to print the database contents for testing.
void print_database(char* database[], int size) {
    printf("{ ");
    for (int i = 0; i < size; i++) {
        if (database[i] != NULL) {
            printf("\"%s\" ", database[i]);
        }
    }
    printf("}\n");
}


int main() {
    // Simulating a database using an array of char pointers (string literals).
    char* subscriberDatabase[MAX_DB_SIZE] = {
        "test@example.com",
        "user1@domain.com",
        "another.user@service.net"
    };
    int current_db_size = 3;

    char result_message[MSG_BUFFER_SIZE];

    printf("Initial database: ");
    print_database(subscriberDatabase, current_db_size);
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Unsubscribe an existing email
    unsubscribe("test@example.com", subscriberDatabase, &current_db_size, result_message, MSG_BUFFER_SIZE);
    printf("1. %s\n", result_message);
    printf("   Database after: ");
    print_database(subscriberDatabase, current_db_size);

    // Test Case 2: Attempt to unsubscribe an email that is not in the database
    unsubscribe("nonexistent@example.com", subscriberDatabase, &current_db_size, result_message, MSG_BUFFER_SIZE);
    printf("2. %s\n", result_message);
    printf("   Database after: ");
    print_database(subscriberDatabase, current_db_size);

    // Test Case 3: Attempt to unsubscribe with an invalid email format
    unsubscribe("invalid-email", subscriberDatabase, &current_db_size, result_message, MSG_BUFFER_SIZE);
    printf("3. %s\n", result_message);
    printf("   Database after: ");
    print_database(subscriberDatabase, current_db_size);

    // Test Case 4: Attempt to unsubscribe another existing email
    unsubscribe("user1@domain.com", subscriberDatabase, &current_db_size, result_message, MSG_BUFFER_SIZE);
    printf("4. %s\n", result_message);
    printf("   Database after: ");
    print_database(subscriberDatabase, current_db_size);

    // Test Case 5: Attempt to unsubscribe with a NULL input
    unsubscribe(NULL, subscriberDatabase, &current_db_size, result_message, MSG_BUFFER_SIZE);
    printf("5. %s\n", result_message);
    printf("   Database after: ");
    print_database(subscriberDatabase, current_db_size);

    return 0;
}