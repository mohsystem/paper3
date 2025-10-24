/*
 * IMPORTANT: This code requires the OpenSSL library for secure password hashing.
 * The C standard library does not provide the necessary cryptographic functions.
 *
 * To compile this code, you need to have OpenSSL installed.
 *
 * Compilation command (on Linux/macOS):
 * gcc -o Task42 Task42.c -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define MAX_USERNAME_LEN 50
#define SALT_LEN 16
#define HASH_LEN 32 // 256 bits for SHA-256
#define ITERATIONS 65536
#define MAX_USERS 100

typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
} UserData;

// In-memory user store
UserData user_store[MAX_USERS];
int user_count = 0;

int hash_password(const char* password, const unsigned char* salt, unsigned char* hash_out) {
    return PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LEN,
        ITERATIONS,
        EVP_sha256(),
        HASH_LEN,
        hash_out
    );
}

UserData* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_store[i].username, username) == 0) {
            return &user_store[i];
        }
    }
    return NULL;
}

bool registerUser(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    if (strlen(username) >= MAX_USERNAME_LEN) {
        fprintf(stderr, "Username too long.\n");
        return false;
    }
    if (user_count >= MAX_USERS) {
        fprintf(stderr, "User store is full.\n");
        return false;
    }
    if (find_user(username) != NULL) {
        return false; // User already exists
    }

    UserData* new_user = &user_store[user_count];
    strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
    new_user->username[MAX_USERNAME_LEN - 1] = '\0';

    if (!RAND_bytes(new_user->salt, SALT_LEN)) {
        fprintf(stderr, "Error generating salt.\n");
        return false;
    }

    if (hash_password(password, new_user->salt, new_user->hash) != 1) {
        fprintf(stderr, "Error hashing password.\n");
        return false;
    }

    user_count++;
    return true;
}

bool loginUser(const char* username, const char* password) {
    UserData* user = find_user(username);
    if (user == NULL) {
        return false; // User not found
    }

    unsigned char attempted_hash[HASH_LEN];
    if (hash_password(password, user->salt, attempted_hash) != 1) {
        fprintf(stderr, "Error hashing password during login attempt.\n");
        return false;
    }

    // Constant-time comparison to prevent timing attacks
    return CRYPTO_memcmp(user->hash, attempted_hash, HASH_LEN) == 0;
}

int main() {
    printf("--- User Authentication System Test ---\n");

    // Test Case 1: Register a new user
    printf("\nTest Case 1: Register a new user 'alice'\n");
    bool registered = registerUser("alice", "Password123!");
    printf("Registration successful: %s\n", registered ? "true" : "false");

    // Test Case 2: Log in with correct credentials
    printf("\nTest Case 2: Log in as 'alice' with correct password\n");
    bool loginSuccess = loginUser("alice", "Password123!");
    printf("Login successful: %s\n", loginSuccess ? "true" : "false");

    // Test Case 3: Log in with incorrect credentials
    printf("\nTest Case 3: Log in as 'alice' with incorrect password\n");
    bool loginFail = loginUser("alice", "WrongPassword");
    printf("Login successful: %s\n", loginFail ? "true" : "false");

    // Test Case 4: Attempt to register an existing user
    printf("\nTest Case 4: Attempt to register existing user 'alice'\n");
    bool reRegistered = registerUser("alice", "AnotherPassword");
    printf("Registration successful: %s\n", reRegistered ? "true" : "false");

    // Test Case 5: Attempt to log in with a non-existent user
    printf("\nTest Case 5: Attempt to log in as non-existent user 'bob'\n");
    bool nonExistentLogin = loginUser("bob", "SomePassword");
    printf("Login successful: %s\n", nonExistentLogin ? "true" : "false");

    return 0;
}