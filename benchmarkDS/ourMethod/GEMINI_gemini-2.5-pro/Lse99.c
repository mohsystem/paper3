/*
 * COMPILE with: gcc -o your_program_name your_source_file.c -lssl -lcrypto
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// NOTE: This C example uses a very simple, non-scalable in-memory "database"
// for demonstration purposes only. A real application would use a proper database.
#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_HEX_LEN 129 // (64 bytes * 2 hex chars) + 1 for null terminator

struct UserRecord {
    char username[MAX_USERNAME_LEN];
    char salt_hex[MAX_HEX_LEN];
    char hash_hex[MAX_HEX_LEN];
};

struct UserRecord userDatabase[MAX_USERS];
int userCount = 0;

// Constants for PBKDF2
#define SALT_LENGTH 16
#define KEY_LENGTH 32 // 256 bits for SHA256
#define ITERATION_COUNT 310000

// Helper function to convert binary data to a hex string
void to_hex(const unsigned char *bin, size_t len, char *hex) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex + (i * 2), "%02x", bin[i]);
    }
    hex[len * 2] = '\0';
}

// Hashes a password using PBKDF2-HMAC-SHA256
int hashPassword(const char *password, unsigned char *salt, unsigned char *hash) {
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt.\n");
        return 0;
    }

    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LENGTH,
        ITERATION_COUNT,
        EVP_sha256(),
        KEY_LENGTH,
        hash
    );

    if (result != 1) {
        fprintf(stderr, "Error: PKCS5_PBKDF2_HMAC failed.\n");
        ERR_print_errors_fp(stderr);
        return 0;
    }
    return 1;
}

// Simulates inserting user credentials into a database
void insertIntoDatabase(const char *username, const char *salt_hex, const char *hash_hex) {
    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Database is full.\n");
        return;
    }
    printf("Simulating database insert for user: %s\n", username);
    strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
    userDatabase[userCount].username[MAX_USERNAME_LEN - 1] = '\0';
    
    strncpy(userDatabase[userCount].salt_hex, salt_hex, MAX_HEX_LEN - 1);
    userDatabase[userCount].salt_hex[MAX_HEX_LEN - 1] = '\0';

    strncpy(userDatabase[userCount].hash_hex, hash_hex, MAX_HEX_LEN - 1);
    userDatabase[userCount].hash_hex[MAX_HEX_LEN - 1] = '\0';

    printf(" -> Stored Salt: %s\n", salt_hex);
    printf(" -> Stored Hash: %s\n", hash_hex);
    printf("User '%s' successfully registered.\n", username);
    userCount++;
}

// Registers a new user
int registerUser(const char *username, char *password) {
    if (username == NULL || strlen(username) == 0 || strspn(username, " \t\n\r") == strlen(username)) {
        fprintf(stderr, "Error: Username cannot be empty.\n");
        return 0;
    }
    if (password == NULL || strlen(password) < 8) {
        fprintf(stderr, "Error: Password must be at least 8 characters long.\n");
        if (password) OPENSSL_cleanse(password, strlen(password));
        return 0;
    }
    for (int i = 0; i < userCount; i++) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            fprintf(stderr, "Error: Username '%s' already exists.\n", username);
            OPENSSL_cleanse(password, strlen(password));
            return 0;
        }
    }

    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];

    if (!hashPassword(password, salt, hash)) {
        fprintf(stderr, "Error: Password hashing failed.\n");
        OPENSSL_cleanse(password, strlen(password));
        return 0;
    }

    // Securely clear the password from memory
    OPENSSL_cleanse(password, strlen(password));

    char salt_hex[SALT_LENGTH * 2 + 1];
    char hash_hex[KEY_LENGTH * 2 + 1];

    to_hex(salt, SALT_LENGTH, salt_hex);
    to_hex(hash, KEY_LENGTH, hash_hex);

    insertIntoDatabase(username, salt_hex, hash_hex);
    return 1;
}


int main() {
    printf("--- Running 5 Test Cases ---\n");
    // Use mutable char arrays for passwords to allow cleansing
    char password_buffer[100];

    printf("\n--- Test Case 1: Valid user ---\n");
    strcpy(password_buffer, "Password123!");
    registerUser("alice", password_buffer);

    printf("\n--- Test Case 2: Another valid user ---\n");
    strcpy(password_buffer, "SecurePass!@#$");
    registerUser("bob", password_buffer);

    printf("\n--- Test Case 3: User with short password ---\n");
    strcpy(password_buffer, "short");
    registerUser("charlie", password_buffer);
    
    printf("\n--- Test Case 4: Duplicate username ---\n");
    strcpy(password_buffer, "AnotherPassword456");
    registerUser("alice", password_buffer);

    printf("\n--- Test Case 5: User with empty username ---\n");
    strcpy(password_buffer, "ValidPassword123");
    registerUser(" ", password_buffer);

    printf("\n--- Test Cases Finished ---\n");
    return 0;
}