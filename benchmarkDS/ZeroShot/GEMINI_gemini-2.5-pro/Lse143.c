#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * A very basic email validation.
 * In a production environment, use a robust library like PCRE for regex matching.
 * This function just checks for the presence and basic placement of '@' and '.'
 * to prevent obviously malformed input.
 * @param email The email string to validate.
 * @return true if the email seems plausible, false otherwise.
 */
bool isValidEmail(const char* email) {
    if (email == NULL || *email == '\0') {
        return false;
    }
    const char* at_char = strchr(email, '@');
    if (at_char == NULL || at_char == email) { // Must have '@' and not at the start
        return false;
    }
    const char* dot_char = strrchr(at_char, '.');
    // Must have '.' after '@', not immediately after, and with chars following
    if (dot_char == NULL || dot_char == at_char + 1 || *(dot_char + 1) == '\0') {
        return false;
    }
    return true;
}

/**
 * Creates a dynamically allocated string for return messages.
 * The caller is responsible for freeing this memory using free().
 * Uses snprintf for safe string formatting to prevent buffer overflows.
 */
char* create_message(const char* format, const char* param) {
    int size = snprintf(NULL, 0, format, param) + 1;
    char* msg = (char*)malloc(size);
    if (msg == NULL) {
        perror("Failed to allocate memory for message");
        exit(EXIT_FAILURE);
    }
    snprintf(msg, size, format, param);
    return msg;
}

/**
 * Unsubscribes a user by removing their email from a simulated database.
 * NOTE: This function returns a dynamically allocated string that the caller MUST free.
 * @param p_database Pointer to the array of strings (the database).
 * @param p_count Pointer to the number of elements in the database.
 * @param email The email of the user to unsubscribe.
 * @return A dynamically allocated status message string.
 */
char* unsubscribe(char*** p_database, int* p_count, const char* email) {
    if (!isValidEmail(email)) {
        return create_message("Error: Invalid email format provided.", ""); // Param is ignored
    }

    char** database = *p_database;
    int count = *p_count;
    int found_index = -1;

    for (int i = 0; i < count; i++) {
        if (strcmp(database[i], email) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Free the memory for the string we are removing
        free(database[found_index]);

        // Shift all subsequent elements one position to the left to fill the gap
        for (int i = found_index; i < count - 1; i++) {
            database[i] = database[i + 1];
        }
        
        (*p_count)--;
        
        return create_message("User with email '%s' has been successfully unsubscribed.", email);
    } else {
        return create_message("User with email '%s' is not subscribed.", email);
    }
}

void printDatabase(char** db, int count) {
    printf("{ ");
    for (int i = 0; i < count; i++) {
        printf("\"%s\" ", db[i]);
    }
    printf("}\n");
}

// Helper to duplicate a string, as strdup is not in standard C.
// Caller is responsible for freeing the returned memory.
char* safe_strdup(const char* s) {
    if (s == NULL) return NULL;
    char* d = malloc(strlen(s) + 1);
    if (d == NULL) return NULL;
    strcpy(d, s);
    return d;
}

int main() {
    // 1. Initialize a simulated database of subscribed emails
    int user_count = 3;
    char** subscribedUsers = (char**)malloc(user_count * sizeof(char*));
    if (subscribedUsers == NULL) {
        perror("Failed to allocate database");
        return EXIT_FAILURE;
    }
    subscribedUsers[0] = safe_strdup("test.user@example.com");
    subscribedUsers[1] = safe_strdup("jane.doe@example.org");
    subscribedUsers[2] = safe_strdup("john.smith@email.net");

    printf("Initial database: ");
    printDatabase(subscribedUsers, user_count);
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Unsubscribe an existing user
    printf("\nTest 1: Unsubscribing an existing user...\n");
    char* result1 = unsubscribe(&subscribedUsers, &user_count, "jane.doe@example.org");
    printf("Result: %s\n", result1);
    printf("Database state: ");
    printDatabase(subscribedUsers, user_count);
    free(result1);

    // Test Case 2: Try to unsubscribe the same user again
    printf("\nTest 2: Unsubscribing the same user again...\n");
    char* result2 = unsubscribe(&subscribedUsers, &user_count, "jane.doe@example.org");
    printf("Result: %s\n", result2);
    printf("Database state: ");
    printDatabase(subscribedUsers, user_count);
    free(result2);

    // Test Case 3: Try to unsubscribe a user not in the database
    printf("\nTest 3: Unsubscribing a non-existent user...\n");
    char* result3 = unsubscribe(&subscribedUsers, &user_count, "not.found@example.com");
    printf("Result: %s\n", result3);
    printf("Database state: ");
    printDatabase(subscribedUsers, user_count);
    free(result3);

    // Test Case 4: Try to unsubscribe with an invalid email format
    printf("\nTest 4: Unsubscribing with an invalid email...\n");
    char* result4 = unsubscribe(&subscribedUsers, &user_count, "invalid-email");
    printf("Result: %s\n", result4);
    printf("Database state: ");
    printDatabase(subscribedUsers, user_count);
    free(result4);
    
    // Test Case 5: Try to unsubscribe with a NULL email
    printf("\nTest 5: Unsubscribing with a NULL email...\n");
    char* result5 = unsubscribe(&subscribedUsers, &user_count, NULL);
    printf("Result: %s\n", result5);
    printf("Database state: ");
    printDatabase(subscribedUsers, user_count);
    free(result5);

    // Cleanup: Free all remaining dynamically allocated memory in the database
    for (int i = 0; i < user_count; i++) {
        free(subscribedUsers[i]);
    }
    free(subscribedUsers);

    return 0;
}