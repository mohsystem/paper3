#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

// In a real application, this would be a persistent database.
// This is a simple in-memory simulation using a dynamic array.
static char** database = NULL;
static size_t db_size = 0;
static size_t db_capacity = 0;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_EMAIL_LENGTH 254

// Forward declarations
void cleanup_database();
bool add_to_database(const char* email);

/**
 * @brief Validates the basic format of an email address.
 *
 * @param email The email string to validate.
 * @return true if the email is in a valid format, false otherwise.
 */
bool is_valid_email(const char* email) {
    if (email == NULL || email[0] == '\0') {
        return false;
    }
    size_t len = strlen(email);
    if (len > MAX_EMAIL_LENGTH) {
        return false;
    }
    // Basic check: must contain '@' and '.'
    const char* at_symbol = strchr(email, '@');
    if (at_symbol == NULL || at_symbol == email) { // must not be the first character
        return false;
    }
    const char* dot_symbol = strrchr(at_symbol, '.');
    if (dot_symbol == NULL || dot_symbol == at_symbol + 1 || dot_symbol[1] == '\0') { // must be after '@' and not the last character
        return false;
    }
    return true;
}

/**
 * @brief Unsubscribes a user by removing their email from the database.
 *
 * @param email The email address of the user to unsubscribe.
 * @return A constant string message indicating the result.
 */
const char* unsubscribe(const char* email) {
    // Rule #1: Ensure all input is validated.
    if (!is_valid_email(email)) {
        return "Error: Invalid email format or parameter.";
    }

    pthread_mutex_lock(&db_mutex);

    int found_index = -1;
    for (size_t i = 0; i < db_size; ++i) {
        if (strcmp(database[i], email) == 0) {
            found_index = (int)i;
            break;
        }
    }

    if (found_index != -1) {
        free(database[found_index]); // Free the string memory
        database[found_index] = NULL;
        
        // Shift remaining elements to fill the gap
        if ((size_t)found_index < db_size - 1) {
            memmove(&database[found_index], &database[found_index + 1], (db_size - 1 - found_index) * sizeof(char*));
        }
        db_size--;

        pthread_mutex_unlock(&db_mutex);
        return "User unsubscribed successfully.";
    } else {
        pthread_mutex_unlock(&db_mutex);
        return "User is not subscribed.";
    }
}

void print_database() {
    printf("   Database state: { ");
    for (size_t i = 0; i < db_size; i++) {
        printf("\"%s\" ", database[i]);
    }
    printf("}\n");
}

int main() {
    // Populate the database for testing
    add_to_database("test@example.com");
    add_to_database("user1@domain.com");
    add_to_database("another.user@service.net");

    printf("Initial database state:\n");
    print_database();
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: A valid email that exists in the database.
    const char* email1 = "test@example.com";
    printf("\n1. Unsubscribing '%s':\n", email1);
    const char* result1 = unsubscribe(email1);
    printf("   Result: %s\n", result1);
    print_database();

    // Test Case 2: The same email, which should now be gone.
    printf("\n2. Unsubscribing '%s' again:\n", email1);
    const char* result2 = unsubscribe(email1);
    printf("   Result: %s\n", result2);

    // Test Case 3: A valid email that is not in the database.
    const char* email3 = "notfound@example.com";
    printf("\n3. Unsubscribing '%s':\n", email3);
    const char* result3 = unsubscribe(email3);
    printf("   Result: %s\n", result3);

    // Test Case 4: An invalid email format.
    const char* email4 = "invalid-email";
    printf("\n4. Unsubscribing '%s':\n", email4);
    const char* result4 = unsubscribe(email4);
    printf("   Result: %s\n", result4);

    // Test Case 5: A NULL email input.
    const char* email5 = NULL;
    printf("\n5. Unsubscribing a NULL email:\n");
    const char* result5 = unsubscribe(email5);
    printf("   Result: %s\n", result5);
    
    printf("\n--- Test Cases Complete ---\n");
    printf("Final database state:\n");
    print_database();

    cleanup_database();
    pthread_mutex_destroy(&db_mutex);
    return 0;
}

// --- Helper functions for database simulation ---

/**
 * @brief Adds a new email to the database, handling memory allocation.
 *
 * @param email The email string to add.
 * @return true on success, false on memory allocation failure.
 */
bool add_to_database(const char* email) {
    pthread_mutex_lock(&db_mutex);

    if (db_size >= db_capacity) {
        size_t new_capacity = (db_capacity == 0) ? 8 : db_capacity * 2;
        char** new_db = realloc(database, new_capacity * sizeof(char*));
        if (new_db == NULL) {
            fprintf(stderr, "Failed to reallocate database memory\n");
            pthread_mutex_unlock(&db_mutex);
            return false;
        }
        database = new_db;
        db_capacity = new_capacity;
    }

    size_t email_len = strlen(email);
    database[db_size] = malloc(email_len + 1);
    if (database[db_size] == NULL) {
        fprintf(stderr, "Failed to allocate memory for email string\n");
        pthread_mutex_unlock(&db_mutex);
        return false;
    }
    memcpy(database[db_size], email, email_len + 1); // Use memcpy for safety
    db_size++;

    pthread_mutex_unlock(&db_mutex);
    return true;
}

/**
 * @brief Frees all memory associated with the database.
 */
void cleanup_database() {
    pthread_mutex_lock(&db_mutex);
    for (size_t i = 0; i < db_size; ++i) {
        free(database[i]);
    }
    free(database);
    database = NULL;
    db_size = 0;
    db_capacity = 0;
    pthread_mutex_unlock(&db_mutex);
}