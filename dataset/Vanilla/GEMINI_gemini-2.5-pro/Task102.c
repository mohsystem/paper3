#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 10
#define USERNAME_LEN 50
#define HASH_LEN 17 // 16 hex chars for 64-bit hash + 1 for null terminator

// NOTE: C does not have a standard library for cryptographic hashing or data structures like maps.
// The hashPassword function below is a VERY SIMPLE, NON-SECURE hash for demonstration purposes only.
// DO NOT USE THIS IN A REAL APPLICATION.
// For production code, use a dedicated library like OpenSSL or libsodium
// to implement a strong, salted hashing algorithm (e.g., bcrypt, Argon2).

typedef struct {
    char username[USERNAME_LEN];
    char hashedPassword[HASH_LEN];
} User;

// Simulates a database table of users using a simple array.
User database[MAX_USERS];
int userCount = 0;

/**
 * A simple, non-secure string hashing function for demonstration.
 * WARNING: NOT FOR PRODUCTION USE.
 * @param password The string to hash.
 * @param hashOutput A buffer to store the resulting hex string hash. Must be at least HASH_LEN bytes.
 */
void hashPassword(const char* password, char* hashOutput) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    sprintf(hashOutput, "%016lx", hash);
}

/**
 * Updates the user's password in the simulated database. If user does not exist, they are created.
 * @param username The username of the user to update.
 * @param newPassword The new plain text password.
 */
void updateUserPassword(const char* username, const char* newPassword) {
    char hashedPassword[HASH_LEN];
    hashPassword(newPassword, hashedPassword);
    
    // Search for existing user
    for (int i = 0; i < userCount; i++) {
        if (strcmp(database[i].username, username) == 0) {
            strcpy(database[i].hashedPassword, hashedPassword);
            printf("Password for user '%s' has been updated.\n", username);
            return;
        }
    }
    
    // If user not found and there's space, add new user
    if (userCount < MAX_USERS) {
        strcpy(database[userCount].username, username);
        strcpy(database[userCount].hashedPassword, hashedPassword);
        userCount++;
        printf("Password for new user '%s' has been set.\n", username);
    } else {
        printf("Error: Database is full. Cannot add user '%s'.\n", username);
    }
}

void printDatabase() {
    for (int i = 0; i < userCount; i++) {
        printf("User: %s, Hashed Pass: %s\n", database[i].username, database[i].hashedPassword);
    }
}

int main() {
    // Initialize database with some users
    updateUserPassword("alice", "initialPass1");
    updateUserPassword("bob", "initialPass2");

    printf("\n--- Initial Database State ---\n");
    printDatabase();
    
    printf("\n--- Running 5 Test Cases ---\n");
    
    // Test Case 1: Update existing user 'alice'
    updateUserPassword("alice", "newSecurePassword123");

    // Test Case 2: Update existing user 'bob'
    updateUserPassword("bob", "anotherPassword!@#");

    // Test Case 3: Add a new user 'charlie'
    updateUserPassword("charlie", "charliePass2024");
    
    // Test Case 4: Update user 'alice' again
    updateUserPassword("alice", "evenNewerPassword$");

    // Test Case 5: Add a new user 'david'
    updateUserPassword("david", "d@v1d$P@$$");

    printf("\n--- Final Database State ---\n");
    printDatabase();

    return 0;
}