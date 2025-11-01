#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 50
#define MAX_LEN 64

typedef struct {
    char username[MAX_LEN];
    char hashedPassword[MAX_LEN];
} User;

// Simulating a database with a global array
static User userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * Hashes a password using a simple, NON-CRYPTOGRAPHICALLY-SECURE algorithm (djb2).
 * NOTE: For a real-world application, link against a proper cryptographic library like OpenSSL.
 * @param password The plain text password.
 * @param hashOutput A buffer to store the resulting hex string hash.
 */
void hashPassword(const char* password, char* hashOutput) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    sprintf(hashOutput, "%lx", hash);
}

/**
 * Registers a new user if the username doesn't already exist.
 * @param username The username for the new account.
 * @param password The password for the new account.
 * @return 1 if registration is successful, 0 otherwise.
 */
int registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return 0;
    }

    // Check if user already exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            printf("Registration failed: Username '%s' already exists.\n", username);
            return 0;
        }
    }

    // Add new user
    char hashedPassword[MAX_LEN];
    hashPassword(password, hashedPassword);
    
    strncpy(userDatabase[userCount].username, username, MAX_LEN - 1);
    userDatabase[userCount].username[MAX_LEN - 1] = '\0'; // Ensure null termination

    strncpy(userDatabase[userCount].hashedPassword, hashedPassword, MAX_LEN - 1);
    userDatabase[userCount].hashedPassword[MAX_LEN - 1] = '\0';

    userCount++;
    printf("User '%s' registered successfully.\n", username);
    return 1;
}

int main() {
    printf("--- C Registration Test ---\n");

    // Test Case 1: Successful registration
    registerUser("alice", "password123");

    // Test Case 2: Successful registration for another user
    registerUser("bob", "securePass!@#");

    // Test Case 3: Attempting to register with an existing username
    registerUser("alice", "anotherPassword");

    // Test Case 4: Successful registration with a complex username
    registerUser("charlie_1990", "my-secret-pw");
    
    // Test Case 5: Successful registration
    registerUser("david", "d@v1dP@$$");
    
    printf("\n--- Current Database State ---\n");
    for (int i = 0; i < userCount; i++) {
        printf("Username: %s, Hashed Password: %s\n", userDatabase[i].username, userDatabase[i].hashedPassword);
    }

    return 0;
}