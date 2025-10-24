#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_STRING_LENGTH 50

// Define a structure to hold user data
typedef struct {
    char username[MAX_STRING_LENGTH];
    char password[MAX_STRING_LENGTH];
} User;

// A simple in-memory array to store user data.
// In a real application, this would be a database.
static User userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * @brief Signs up a new user by storing their username and password.
 * NOTE: In a real-world application, passwords should NEVER be stored in plaintext.
 * They should be securely hashed.
 *
 * @param username The username to register.
 * @param password The password for the new user.
 * @return true if registration is successful, false if username exists or DB is full.
 */
bool signUp(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0) {
        printf("Signup failed: Username cannot be empty.\n");
        return false;
    }
    
    // Check if the database is full
    if (userCount >= MAX_USERS) {
        printf("Signup failed: User database is full.\n");
        return false;
    }

    // Check if the username already exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            printf("Signup failed: Username '%s' already exists.\n", username);
            return false;
        }
    }

    // Add the new user
    strncpy(userDatabase[userCount].username, username, MAX_STRING_LENGTH - 1);
    userDatabase[userCount].username[MAX_STRING_LENGTH - 1] = '\0'; // Ensure null-termination
    
    strncpy(userDatabase[userCount].password, password, MAX_STRING_LENGTH - 1);
    userDatabase[userCount].password[MAX_STRING_LENGTH - 1] = '\0'; // Ensure null-termination

    userCount++;
    printf("Signup successful: User '%s' created.\n", username);
    return true;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Successful signup for a new user
    signUp("alice", "password123");

    // Test Case 2: Successful signup for another new user
    signUp("bob", "securePass_456");

    // Test Case 3: Failed signup with an existing username
    signUp("alice", "anotherPassword");

    // Test Case 4: Successful signup with a user whose name is a number
    signUp("user789", "mySecret");

    // Test Case 5: Successful signup with an empty password (for demonstration)
    signUp("charlie", "");

    // Print final state of the database
    printf("\nCurrent user database:\n");
    for (int i = 0; i < userCount; i++) {
        printf("  - User %d: Username=%s, Password=%s\n", i + 1, userDatabase[i].username, userDatabase[i].password);
    }

    return 0;
}