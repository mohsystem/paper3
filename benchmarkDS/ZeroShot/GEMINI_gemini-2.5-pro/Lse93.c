/*
To compile this code, you need the OpenSSL library installed.
Compile using gcc:
gcc your_file_name.c -o Lse93 -lcrypto

This code uses PBKDF2 with SHA256 for password hashing, which is a secure standard.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_USERS 10
#define USERNAME_MAX_LEN 50
#define STORED_HASH_MAX_LEN 257 // salt(32) + '$' + hash(64) + null terminator

// Constants for PBKDF2
#define SALT_LEN 16
#define KEY_LEN 32 // 256 bits for SHA256
#define ITERATIONS 100000

// Structure to hold user data
struct User {
    char username[USERNAME_MAX_LEN];
    char stored_hash[STORED_HASH_MAX_LEN];
};

// A simple in-memory array to simulate a database.
static struct User userDatabase[MAX_USERS];
static int userCount = 0;

// Helper function to convert binary bytes to a hex string
void to_hex(const unsigned char* input, size_t len, char* output) {
    for (size_t i = 0; i < len; i++) {
        sprintf(output + (i * 2), "%02x", input[i]);
    }
}

/**
 * Hashes a password and stores the user record in a simulated database.
 * The stored format is "salt_hex$hash_hex".
 *
 * @param username The username to register.
 * @param password The plain-text password to hash.
 */
void registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        fprintf(stderr, "Error: Username and password cannot be empty.\n");
        return;
    }
    if (strlen(username) >= USERNAME_MAX_LEN) {
        fprintf(stderr, "Error: Username is too long.\n");
        return;
    }
    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Error: Database is full.\n");
        return;
    }

    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            fprintf(stderr, "Registration failed: Username '%s' already exists.\n", username);
            return;
        }
    }

    // 1. Generate a random salt
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating salt.\n");
        return;
    }

    // 2. Hash the password using PBKDF2
    unsigned char key[KEY_LEN];
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(key),
        key
    );

    if (result != 1) {
        fprintf(stderr, "Error in PKCS5_PBKDF2_HMAC.\n");
        return;
    }

    // 3. Convert salt and hash to hex and store them
    char salt_hex[SALT_LEN * 2 + 1];
    char key_hex[KEY_LEN * 2 + 1];
    to_hex(salt, sizeof(salt), salt_hex);
    to_hex(key, sizeof(key), key_hex);

    // Securely copy data to our database struct
    strncpy(userDatabase[userCount].username, username, USERNAME_MAX_LEN - 1);
    userDatabase[userCount].username[USERNAME_MAX_LEN - 1] = '\0'; // Ensure null termination
    
    snprintf(userDatabase[userCount].stored_hash, STORED_HASH_MAX_LEN, "%s$%s", salt_hex, key_hex);

    userCount++;
    printf("User '%s' registered successfully.\n", username);
}

int main() {
    printf("--- Running 5 Registration Test Cases ---\n");

    // Test Case 1: Standard registration
    registerUser("alice", "Password123!");

    // Test Case 2: Another standard registration
    registerUser("bob", "my$ecretP@ss");

    // Test Case 3: Registration with a different password
    registerUser("charlie", "Ch@rlieR0cks");

    // Test Case 4: Attempting to register a user that already exists
    registerUser("alice", "another_password");

    // Test Case 5: Registration with a short password
    registerUser("david", "p@ss");
    
    printf("\n--- Simulated Database Contents ---\n");
    for (int i = 0; i < userCount; i++) {
        printf("Username: %s, Stored Hash: %s\n", userDatabase[i].username, userDatabase[i].stored_hash);
    }

    return 0;
}