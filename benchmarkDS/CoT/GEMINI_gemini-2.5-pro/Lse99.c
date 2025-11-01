#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 10
#define USERNAME_LEN 50
#define STORED_PASS_LEN 256

// Simulate a database using an array of structs
typedef struct {
    char username[USERNAME_LEN];
    char stored_password[STORED_PASS_LEN];
} User;

User userDatabase[MAX_USERS];
int userCount = 0;

/**
 * Registers a user by creating a placeholder for their hashed password.
 * @param username The username to register.
 * @param password The plain-text password.
 */
void registerUser(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        printf("Error: Username or password cannot be empty.\n");
        return;
    }
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            printf("Error: Username '%s' already exists.\n", username);
            return;
        }
    }
    if (userCount >= MAX_USERS) {
        printf("Error: Database is full. Cannot register new user.\n");
        return;
    }

    /*
     * ========================= IMPORTANT SECURITY NOTE =========================
     * The C standard library does not provide cryptographic functions. Securely
     * hashing passwords requires a dedicated external library like OpenSSL
     * (using its libcrypto) or libsodium. These libraries provide functions
     * for algorithms like bcrypt, scrypt, or Argon2 which are specifically
     * designed to be slow and protect against brute-force attacks.
     *
     * DO NOT use simple hashes like MD5/SHA-1 for passwords.
     *
     * The code below is a NON-FUNCTIONAL PLACEHOLDER. A real implementation would:
     * 1. Use a function like `RAND_bytes()` from OpenSSL to generate a salt.
     * 2. Use a key derivation function like `PKCS5_PBKDF2_HMAC()` from OpenSSL
     *    to hash the password with the salt.
     * 3. Store the salt and the resulting hash in the database.
     * =========================================================================
     */
    const char* stored_value = "PLACEHOLDER_FOR_SALT_AND_HASH_FROM_A_CRYPTO_LIBRARY";

    // "Insert" into the database
    strncpy(userDatabase[userCount].username, username, USERNAME_LEN - 1);
    userDatabase[userCount].username[USERNAME_LEN - 1] = '\0'; // Ensure null termination

    strncpy(userDatabase[userCount].stored_password, stored_value, STORED_PASS_LEN - 1);
    userDatabase[userCount].stored_password[STORED_PASS_LEN - 1] = '\0'; // Ensure null termination
    
    printf("User '%s' registered successfully (with placeholder hash).\n", username);
    userCount++;
}

/**
 * Prints the contents of the simulated user database.
 */
void printDatabase() {
    printf("\n--- Simulated User Database ---\n");
    if (userCount == 0) {
        printf("Database is empty.\n");
    } else {
        for (int i = 0; i < userCount; i++) {
            printf("Username: %s, Stored Hash: %s\n", userDatabase[i].username, userDatabase[i].stored_password);
        }
    }
    printf("-----------------------------\n\n");
}

int main() {
    // --- 5 Test Cases ---
    printf("--- Running C Test Cases ---\n");
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "Ch@rlieBrown");
    registerUser("david", "d@v1d_the_k1ng");
    registerUser("eve", "EveIsAwesome#2024");

    // Test case for existing user
    registerUser("alice", "AnotherPassword");

    // Print the final state of the database
    printDatabase();

    return 0;
}