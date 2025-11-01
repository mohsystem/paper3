// This code requires the OpenSSL library.
// Compile with: gcc your_file.c -o your_app -lssl -lcrypto
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// OpenSSL headers for hashing and random number generation
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants
#define SALT_BYTES 16
#define HASH_BYTES 32 // 256 bits for SHA256
#define ITERATIONS 100000

#define MAX_USERS 10
#define USERNAME_MAX_LEN 50
// Stored format: hex(salt) + ':' + hex(hash) + '\0'
// hex(salt)=16*2=32, hex(hash)=32*2=64, separator=1, null=1 -> Total 98
#define STORED_PASS_LEN (SALT_BYTES * 2 + HASH_BYTES * 2 + 2)

// Simulated database entry
typedef struct {
    char username[USERNAME_MAX_LEN];
    char stored_password[STORED_PASS_LEN];
} User;

// Simulated in-memory database
static User database[MAX_USERS];
static int user_count = 0;

/**
 * Helper function to convert binary data to a hex string.
 * Ensures the destination buffer is large enough.
 */
void to_hex(char* dest, const unsigned char* src, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(dest + (i * 2), "%02x", src[i]);
    }
    dest[len * 2] = '\0'; // Null-terminate the string
}

/**
 * Finds a user by username in the database.
 * @return The index of the user if found, otherwise -1.
 */
int find_user_by_name(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(database[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Registers a new user by securely hashing their password.
 * @param username The username.
 * @param password The plaintext password.
 * @return 1 on success, 0 on failure.
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        fprintf(stderr, "Error: Username and password cannot be empty.\n");
        return 0;
    }
    if (strlen(username) >= USERNAME_MAX_LEN) {
        fprintf(stderr, "Error: Username is too long.\n");
        return 0;
    }
    if (user_count >= MAX_USERS) {
        fprintf(stderr, "Error: Database is full.\n");
        return 0;
    }
    if (find_user_by_name(username) != -1) {
        fprintf(stderr, "Error: Username '%s' already exists.\n", username);
        return 0;
    }

    // 1. Generate salt
    unsigned char salt[SALT_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Critical Error: Failed to generate random salt.\n");
        return 0;
    }

    // 2. Hash password using PBKDF2-HMAC-SHA256
    unsigned char hash[HASH_BYTES];
    int result = PKCS5_PBKDF2_HMAC(
        password, strlen(password),
        salt, sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash), hash
    );

    if (result != 1) {
        fprintf(stderr, "Critical Error: Failed to hash password.\n");
        return 0;
    }
    
    // 3. Format and store the salt and hash
    char salt_hex[SALT_BYTES * 2 + 1];
    char hash_hex[HASH_BYTES * 2 + 1];
    to_hex(salt_hex, salt, sizeof(salt));
    to_hex(hash_hex, hash, sizeof(hash));
    
    User* new_user = &database[user_count];
    strncpy(new_user->username, username, USERNAME_MAX_LEN - 1);
    new_user->username[USERNAME_MAX_LEN - 1] = '\0';
    
    snprintf(new_user->stored_password, STORED_PASS_LEN, "%s:%s", salt_hex, hash_hex);
    
    user_count++;
    
    printf("User '%s' registered successfully.\n", username);
    return 1;
}

int main(void) {
    printf("--- C Registration Test ---\n");
    
    // Test Case 1: Successful registration
    registerUser("alice", "Password123!");
    
    // Test Case 2: Successful registration
    registerUser("bob", "SecurePass@2023");
    
    // Test Case 3: Attempt with an empty password
    registerUser("charlie", "");
    
    // Test Case 4: Attempt to register a duplicate username
    registerUser("alice", "AnotherPassword");
    
    // Test Case 5: Successful registration with complex password
    registerUser("diane", "d!ane_p@ssw0rd_c");

    printf("\n--- Simulated Database Contents ---\n");
    for (int i = 0; i < user_count; ++i) {
        printf("Username: %s, Stored Value: %s\n", database[i].username, database[i].stored_password);
    }
    printf("---------------------------------\n");

    return 0;
}