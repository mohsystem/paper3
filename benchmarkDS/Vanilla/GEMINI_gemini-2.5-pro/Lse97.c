#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 50
#define MAX_STRING_LEN 100
#define HASH_STRING_LEN 17 // 16 hex chars + null terminator

// User struct to hold user data
typedef struct {
    char username[MAX_STRING_LEN];
    char hashedPassword[HASH_STRING_LEN];
    char firstName[MAX_STRING_LEN];
    char lastName[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
} User;

// Array of structs to simulate a database
User database[MAX_USERS];
int userCount = 0;

/**
 * A simple, non-cryptographic "hashing" function for demonstration purposes.
 * In a real application, use a proper cryptographic library like OpenSSL.
 * This function calculates a DJB2 hash and returns it as a hex string.
 * @param password The password to hash.
 * @param hashedOutput Buffer to store the resulting hash string.
 */
void hashPassword(const char* password, char* hashedOutput) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    snprintf(hashedOutput, HASH_STRING_LEN, "%016lx", hash);
}

/**
 * Registers a new user.
 * @param username The username.
 * @param password The plain text password.
 * @param firstName The user's first name.
 * @param lastName The user's last name.
 * @param email The user's email.
 * @return A dynamically allocated message string indicating success or failure. The caller must free this string.
 */
char* registerUser(const char* username, const char* password, const char* firstName, const char* lastName, const char* email) {
    // Check if user exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(database[i].username, username) == 0) {
            char* message = (char*)malloc(MAX_STRING_LEN * sizeof(char));
            snprintf(message, MAX_STRING_LEN, "Username '%s' already exists.", username);
            return message;
        }
    }

    // Check if database is full
    if (userCount >= MAX_USERS) {
        char* message = (char*)malloc(MAX_STRING_LEN * sizeof(char));
        strcpy(message, "Database is full. Cannot register new user.");
        return message;
    }

    // Add new user
    char hashedPassword[HASH_STRING_LEN];
    hashPassword(password, hashedPassword);
    
    strcpy(database[userCount].username, username);
    strcpy(database[userCount].hashedPassword, hashedPassword);
    strcpy(database[userCount].firstName, firstName);
    strcpy(database[userCount].lastName, lastName);
    strcpy(database[userCount].email, email);
    
    userCount++;

    char* message = (char*)malloc(MAX_STRING_LEN * sizeof(char));
    snprintf(message, MAX_STRING_LEN, "Registration succeeded for username '%s'.", username);
    return message;
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Successful registration
    char* result1 = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
    printf("Test Case 1: %s\n", result1);
    free(result1);

    // Test Case 2: Another successful registration
    char* result2 = registerUser("asmith", "SecurePass!", "Alice", "Smith", "asmith@example.com");
    printf("Test Case 2: %s\n", result2);
    free(result2);

    // Test Case 3: Attempt to register with an existing username
    char* result3 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
    printf("Test Case 3: %s\n", result3);
    free(result3);

    // Test Case 4: A third successful registration
    char* result4 = registerUser("peterpan", "neverland", "Peter", "Pan", "peter.p@example.com");
    printf("Test Case 4: %s\n", result4);
    free(result4);

    // Test Case 5: Another failed registration with a duplicate username
    char* result5 = registerUser("asmith", "MyPass123", "Adam", "Smithy", "a.smith@example.com");
    printf("Test Case 5: %s\n", result5);
    free(result5);
    
    // Optional: Print the database content to verify
    // printf("\nCurrent database state (%d users):\n", userCount);
    // for (int i = 0; i < userCount; i++) {
    //     printf("User: %s, Hashed Pwd: %s\n", database[i].username, database[i].hashedPassword);
    // }

    return 0;
}