/*
 * Before compiling, ensure you have OpenSSL development libraries installed.
 * For example, on Debian/Ubuntu: sudo apt-get install libssl-dev
 *
 * Compile using gcc:
 * gcc your_file_name.c -o auth_service -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// --- Constants ---
#define SALT_LENGTH 16
#define KEY_LENGTH 32 // 256 bits
#define ITERATIONS 400000
#define MIN_PASSWORD_LENGTH 8
#define MAX_USERNAME_LEN 50
#define MAX_USERS 100

typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];
} UserCredentials;

// In-memory user database
static UserCredentials user_database[MAX_USERS];
static int user_count = 0;

// Forward declaration
const UserCredentials* find_user(const char* username);

bool register_user(const char* username, const char* password) {
    if (username == NULL || username[0] == '\0' || strlen(username) >= MAX_USERNAME_LEN) {
        fprintf(stderr, "Error: Invalid username.\n");
        return false;
    }
    if (password == NULL || strlen(password) < MIN_PASSWORD_LENGTH) {
        fprintf(stderr, "Error: Password does not meet policy.\n");
        return false;
    }
    if (user_count >= MAX_USERS) {
        fprintf(stderr, "Error: User database is full.\n");
        return false;
    }
    if (find_user(username) != NULL) {
        fprintf(stderr, "Error: Username '%s' already exists.\n", username);
        return false;
    }

    UserCredentials* new_user = &user_database[user_count];
    strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
    new_user->username[MAX_USERNAME_LEN - 1] = '\0';

    if (RAND_bytes(new_user->salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt.\n");
        return false;
    }

    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        new_user->salt,
        SALT_LENGTH,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        new_user->hash
    );

    if (result != 1) {
        fprintf(stderr, "Error: Failed to hash password with PBKDF2.\n");
        // Clear partially created user entry on failure
        memset(new_user, 0, sizeof(UserCredentials));
        return false;
    }

    user_count++;
    return true;
}

bool authenticate_user(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return false;
    }

    const UserCredentials* user = find_user(username);
    if (user == NULL) {
        return false; // User not found
    }

    unsigned char calculated_hash[KEY_LENGTH];
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        user->salt,
        SALT_LENGTH,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        calculated_hash
    );

    if (result != 1) {
        fprintf(stderr, "Error during password verification.\n");
        OPENSSL_cleanse(calculated_hash, KEY_LENGTH);
        return false;
    }

    // Constant-time comparison
    bool success = (CRYPTO_memcmp(user->hash, calculated_hash, KEY_LENGTH) == 0);
    
    // Securely clear the calculated hash from memory
    OPENSSL_cleanse(calculated_hash, KEY_LENGTH);

    return success;
}

const UserCredentials* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strncmp(user_database[i].username, username, MAX_USERNAME_LEN) == 0) {
            return &user_database[i];
        }
    }
    return NULL;
}

int main(void) {
    // Test Case 1: Register a new user
    printf("Test Case 1: Registering user 'alice'.\n");
    bool registered = register_user("alice", "Password123!");
    printf("Registration successful: %s\n", registered ? "true" : "false");
    printf("--------------------\n");

    // Test Case 2: Authenticate user with correct password
    printf("Test Case 2: Authenticating 'alice' with correct password.\n");
    bool authenticated = authenticate_user("alice", "Password123!");
    printf("Authentication successful: %s\n", authenticated ? "true" : "false");
    printf("--------------------\n");

    // Test Case 3: Authenticate user with incorrect password
    printf("Test Case 3: Authenticating 'alice' with incorrect password.\n");
    authenticated = authenticate_user("alice", "WrongPassword!");
    printf("Authentication successful: %s\n", authenticated ? "true" : "false");
    printf("--------------------\n");

    // Test Case 4: Authenticate a non-existent user
    printf("Test Case 4: Authenticating non-existent user 'bob'.\n");
    authenticated = authenticate_user("bob", "SomePassword");
    printf("Authentication successful: %s\n", authenticated ? "true" : "false");
    printf("--------------------\n");

    // Test Case 5: Register another user and authenticate
    printf("Test Case 5: Registering and authenticating 'charlie'.\n");
    registered = register_user("charlie", "SecurePass!@#");
    printf("Registration of 'charlie' successful: %s\n", registered ? "true" : "false");
    authenticated = authenticate_user("charlie", "SecurePass!@#");
    printf("Authentication of 'charlie' successful: %s\n", authenticated ? "true" : "false");
    printf("--------------------\n");

    // Test Case 6: Attempt to register with a short password
    printf("Test Case 6: Attempt to register with a short password.\n");
    registered = register_user("david", "short");
    printf("Registration successful: %s\n", registered ? "true" : "false");
    printf("--------------------\n");

    // Test Case 7: Attempt to register an existing user
    printf("Test Case 7: Attempt to register 'alice' again.\n");
    registered = register_user("alice", "AnotherPassword");
    printf("Registration successful: %s\n", registered ? "true" : "false");
    printf("--------------------\n");
    
    return 0;
}