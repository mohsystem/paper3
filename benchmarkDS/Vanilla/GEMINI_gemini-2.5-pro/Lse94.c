#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// IMPORTANT: This code requires the OpenSSL library.
// To compile on Linux/macOS: gcc your_file_name.c -o your_program_name -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

#define SALT_SIZE 16
#define MAX_USERNAME_LEN 50
#define MAX_USERS 10

// Struct to hold user data
typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char salt[SALT_SIZE];
    unsigned char hashedPassword[SHA256_DIGEST_LENGTH];
} UserData;

// Simulate a database with a fixed-size array
UserData database[MAX_USERS];
int userCount = 0;

// Helper to print bytes as a hex string
void printHex(const unsigned char* bytes, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", bytes[i]);
    }
}

/**
 * Generates a salt, hashes the password, and "inserts" into the database.
 * @param username The username to register.
 * @param password The plain-text password.
 * @return 0 on success, -1 on failure.
 */
int registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Database is full. Cannot register new user.\n");
        return -1;
    }
    if (strlen(username) >= MAX_USERNAME_LEN) {
        fprintf(stderr, "Username is too long.\n");
        return -1;
    }

    // 1. Generate a random salt
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate random salt.\n");
        return -1;
    }

    // 2. Combine salt and password
    size_t passwordLen = strlen(password);
    size_t toHashLen = SALT_SIZE + passwordLen;
    unsigned char* toHash = malloc(toHashLen);
    if (toHash == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return -1;
    }
    memcpy(toHash, salt, SALT_SIZE);
    memcpy(toHash + SALT_SIZE, password, passwordLen);

    // 3. Hash the combined bytes using SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(toHash, toHashLen, hash);
    free(toHash); // Clean up allocated memory

    // 4. Store user data in the database
    strncpy(database[userCount].username, username, MAX_USERNAME_LEN - 1);
    database[userCount].username[MAX_USERNAME_LEN - 1] = '\0'; // Ensure null termination
    memcpy(database[userCount].salt, salt, SALT_SIZE);
    memcpy(database[userCount].hashedPassword, hash, SHA256_DIGEST_LENGTH);
    
    printf("User '%s' registered successfully.\n", username);
    printf("  - Salt (Hex): ");
    printHex(salt, SALT_SIZE);
    printf("\n");
    printf("  - Hashed Password (Hex): ");
    printHex(hash, SHA256_DIGEST_LENGTH);
    printf("\n");
    printf("-------------------------------------------------\n");
    
    userCount++;
    return 0;
}

// In C, a main class name is not applicable.
// We use a function to represent the main execution block as requested.
void Lse94_main() {
    printf("--- Running 5 Test Cases for User Registration ---\n\n");
    
    // Test Case 1
    registerUser("alice", "password123");

    // Test Case 2
    registerUser("bob", "my$ecretP@ss");

    // Test Case 3
    registerUser("charlie", "CharlieBrown");

    // Test Case 4
    registerUser("dave", "davespassword");

    // Test Case 5: Registering a user with the same password as alice
    registerUser("eve", "password123");

    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < userCount; ++i) {
        printf("Username: %s, HashedPassword: ", database[i].username);
        printHex(database[i].hashedPassword, SHA256_DIGEST_LENGTH);
        printf("\n");
    }
}

int main() {
    Lse94_main();
    return 0;
}