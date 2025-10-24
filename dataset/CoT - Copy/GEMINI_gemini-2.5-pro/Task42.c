/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against it. For example:
 * gcc Task42.c -o task42 -lssl -lcrypto
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

#define SALT_LENGTH 16
#define HASH_LENGTH 32 // SHA-256
#define MAX_USERS 100

typedef struct {
    char* username;
    unsigned char* salt;
    unsigned char* hashedPassword;
} UserCredentials;

// In-memory storage. A real app would use a database.
static UserCredentials userStore[MAX_USERS];
static int user_count = 0;

// Helper to find a user by username
UserCredentials* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(userStore[i].username, username) == 0) {
            return &userStore[i];
        }
    }
    return NULL;
}

// Hashes the password with a given salt using SHA-256.
unsigned char* hashPassword(const char* password, const unsigned char* salt) {
    unsigned char* hash = (unsigned char*)malloc(HASH_LENGTH);
    if (!hash) return NULL;
    
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned int hash_len;

    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, salt, SALT_LENGTH);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    if (hash_len != HASH_LENGTH) {
        free(hash);
        return NULL;
    }

    return hash;
}

// Registers a new user.
int registerUser(const char* username, const char* password) {
    if (user_count >= MAX_USERS) return 0; // Store is full
    if (find_user(username) != NULL) return 0; // User already exists

    // Generate salt
    unsigned char* salt = (unsigned char*)malloc(SALT_LENGTH);
    if (!salt || RAND_bytes(salt, SALT_LENGTH) != 1) {
        free(salt);
        return 0; // Error generating salt
    }

    // Hash password
    unsigned char* hashedPassword = hashPassword(password, salt);
    if (!hashedPassword) {
        free(salt);
        return 0;
    }

    // Store user
    userStore[user_count].username = strdup(username);
    userStore[user_count].salt = salt;
    userStore[user_count].hashedPassword = hashedPassword;
    user_count++;
    
    return 1; // Success
}

// Authenticates a user.
int loginUser(const char* username, const char* password) {
    UserCredentials* user = find_user(username);
    if (user == NULL) {
        return 0; // User not found
    }

    unsigned char* providedPasswordHash = hashPassword(password, user->salt);
    if (!providedPasswordHash) {
        return 0;
    }

    // Constant-time comparison
    int result = CRYPTO_memcmp(user->hashedPassword, providedPasswordHash, HASH_LENGTH) == 0;

    free(providedPasswordHash);
    return result;
}

// Clean up allocated memory
void cleanup() {
    for (int i = 0; i < user_count; ++i) {
        free(userStore[i].username);
        free(userStore[i].salt);
        free(userStore[i].hashedPassword);
    }
}

int main() {
    printf("--- C User Authentication System Test ---\n");

    // Test Case 1: Successful registration
    printf("Test 1: Registering a new user 'alice'.\n");
    int regSuccess = registerUser("alice", "Password123!");
    printf("Registration successful: %s\n\n", regSuccess ? "true" : "false");

    // Test Case 2: Successful login
    printf("Test 2: 'alice' attempts to log in with correct password.\n");
    int loginSuccess = loginUser("alice", "Password123!");
    printf("Login successful: %s\n\n", loginSuccess ? "true" : "false");

    // Test Case 3: Failed login (wrong password)
    printf("Test 3: 'alice' attempts to log in with incorrect password.\n");
    int loginFailWrongPass = loginUser("alice", "WrongPassword!");
    printf("Login successful: %s\n\n", loginFailWrongPass ? "true" : "false");

    // Test Case 4: Failed login (non-existent user)
    printf("Test 4: Non-existent user 'bob' attempts to log in.\n");
    int loginFailNoUser = loginUser("bob", "SomePassword");
    printf("Login successful: %s\n\n", loginFailNoUser ? "true" : "false");

    // Test Case 5: Failed registration (duplicate username)
    printf("Test 5: Attempting to register 'alice' again.\n");
    int regFailDuplicate = registerUser("alice", "AnotherPassword");
    printf("Registration successful: %s\n\n", regFailDuplicate ? "true" : "false");

    cleanup();
    return 0;
}