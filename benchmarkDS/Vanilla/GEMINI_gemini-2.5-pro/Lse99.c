#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define HASHED_PASSWORD_LEN 21 // Sufficient for unsigned long string + null terminator

// Represents a user record in the "database"
struct User {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[HASHED_PASSWORD_LEN];
};

// Simulating a database table with a static array
struct User database[MAX_USERS];
int user_count = 0;

/**
 * A simple, non-cryptographically secure hashing function for demonstration.
 * In a real application, use a proper library like OpenSSL or libsodium.
 * This function uses the djb2 algorithm.
 * @param password The password to hash.
 * @param hashed_output The buffer to store the string representation of the hash.
 */
void hashPassword(const char* password, char* hashed_output) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    sprintf(hashed_output, "%lu", hash);
}

/**
 * "Inserts" a new user into the simulated database.
 * @param username The username for the new user.
 * @param password The password for the new user.
 * @return 1 if the user was successfully registered, 0 otherwise.
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        printf("Error: Username and password cannot be empty.\n");
        return 0;
    }
    if (user_count >= MAX_USERS) {
        printf("Error: Database is full. Cannot register new user.\n");
        return 0;
    }
    if (strlen(username) >= MAX_USERNAME_LEN) {
        printf("Error: Username is too long.\n");
        return 0;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(database[i].username, username) == 0) {
            printf("Error: Username '%s' already exists.\n", username);
            return 0;
        }
    }
    
    char hashedPassword[HASHED_PASSWORD_LEN];
    hashPassword(password, hashedPassword);
    
    strncpy(database[user_count].username, username, MAX_USERNAME_LEN - 1);
    database[user_count].username[MAX_USERNAME_LEN - 1] = '\0'; // Ensure null-termination

    strncpy(database[user_count].hashedPassword, hashedPassword, HASHED_PASSWORD_LEN - 1);
    database[user_count].hashedPassword[HASHED_PASSWORD_LEN - 1] = '\0'; // Ensure null-termination

    user_count++;
    
    printf("User '%s' registered successfully.\n", username);
    return 1;
}

int main() {
    printf("--- Running 5 User Registration Test Cases ---\n");

    // Test Case 1
    registerUser("alice", "password123");

    // Test Case 2
    registerUser("bob", "my$ecretP@ss");

    // Test Case 3
    registerUser("charlie", "Charlie'sPassword!");

    // Test Case 4
    registerUser("david", "d@v1d_R0ck$");

    // Test Case 5
    registerUser("eve", "spyware");

    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < user_count; i++) {
        printf("Username: %s, Hashed Password: %s\n", database[i].username, database[i].hashedPassword);
    }

    return 0;
}