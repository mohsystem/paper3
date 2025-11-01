#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp and OPENSSL_cleanse

// To compile: gcc -Wall your_file_name.c -o your_program -lssl -lcrypto

#define SALT_BYTES 16
#define KEY_BYTES 32 // 256 bits
#define ITERATIONS 600000
#define MAX_USERS 10

typedef struct {
    char* username;
    unsigned char salt[SALT_BYTES];
    unsigned char hash[KEY_BYTES];
} UserEntry;

// In-memory user database simulation
static UserEntry user_database[MAX_USERS];
static int user_count = 0;

void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int hash_password(const char* password, const unsigned char* salt, unsigned char* hash_out) {
    if (!password || !salt || !hash_out) {
        return 0; // Invalid arguments
    }
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_BYTES,
        ITERATIONS,
        EVP_sha256(),
        KEY_BYTES,
        hash_out
    );
    return result;
}

bool register_user(const char* username, char* password) {
    if (username == NULL || strlen(username) == 0) {
        fprintf(stderr, "Error: Username cannot be empty.\n");
        return false;
    }
    if (password == NULL || strlen(password) < 8) {
        fprintf(stderr, "Error: Password must be at least 8 characters long.\n");
        return false;
    }
    if (user_count >= MAX_USERS) {
        fprintf(stderr, "Error: Database is full.\n");
        return false;
    }

    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            fprintf(stderr, "Error: Username '%s' already exists.\n", username);
            return false;
        }
    }

    UserEntry* new_user = &user_database[user_count];
    
    // Generate salt
    if (RAND_bytes(new_user->salt, SALT_BYTES) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt.\n");
        return false;
    }

    // Hash password
    if (hash_password(password, new_user->salt, new_user->hash) != 1) {
        fprintf(stderr, "Error: Failed to hash password.\n");
        return false;
    }

    // Securely clear password from memory
    OPENSSL_cleanse(password, strlen(password));

    // Store user
    new_user->username = strdup(username);
    if (new_user->username == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for username.\n");
        // Clear sensitive data before returning
        OPENSSL_cleanse(new_user, sizeof(UserEntry));
        return false;
    }
    
    user_count++;
    printf("User '%s' registered successfully.\n", username);
    return true;
}

bool verify_password(const char* username, char* password) {
    UserEntry* user = NULL;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            user = &user_database[i];
            break;
        }
    }

    if (user == NULL || password == NULL) {
        return false;
    }

    unsigned char hash_to_verify[KEY_BYTES];
    if (hash_password(password, user->salt, hash_to_verify) != 1) {
        fprintf(stderr, "Error: Failed to hash password for verification.\n");
        return false;
    }
    
    // Securely clear password from memory
    OPENSSL_cleanse(password, strlen(password));

    // Constant-time comparison
    int diff = CRYPTO_memcmp(user->hash, hash_to_verify, KEY_BYTES);
    return (diff == 0);
}

void cleanup_database() {
    for (int i = 0; i < user_count; ++i) {
        free(user_database[i].username);
        user_database[i].username = NULL;
    }
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    char pass_buffer[256];

    // Test Case 1: Successful registration and login
    printf("\n--- Test Case 1: Successful Registration & Login ---\n");
    strncpy(pass_buffer, "Password123!", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    register_user("alice", pass_buffer);

    strncpy(pass_buffer, "Password123!", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    printf("Verifying 'alice' with correct password: %s\n", verify_password("alice", pass_buffer) ? "SUCCESS" : "FAILURE");

    if (user_count > 0 && strcmp(user_database[0].username, "alice") == 0) {
        print_hex("Stored Salt (Hex): ", user_database[0].salt, SALT_BYTES);
        print_hex("Stored Hash (Hex): ", user_database[0].hash, KEY_BYTES);
    }


    // Test Case 2: Failed login attempt with wrong password
    printf("\n--- Test Case 2: Failed Login ---\n");
    strncpy(pass_buffer, "WrongPassword!", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    printf("Verifying 'alice' with incorrect password: %s\n", verify_password("alice", pass_buffer) ? "SUCCESS" : "FAILURE");

    // Test Case 3: Attempt to register a user that already exists
    printf("\n--- Test Case 3: Duplicate Registration ---\n");
    strncpy(pass_buffer, "AnotherPassword456!", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    register_user("alice", pass_buffer);

    // Test Case 4: Attempt to register with a weak password
    printf("\n--- Test Case 4: Weak Password Registration ---\n");
    strncpy(pass_buffer, "pass", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    register_user("bob", pass_buffer);

    // Test Case 5: Successful registration of a second user
    printf("\n--- Test Case 5: Successful Registration of Another User ---\n");
    strncpy(pass_buffer, "S3cureP@ssw0rdF0rCharlie", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    register_user("charlie", pass_buffer);
    
    strncpy(pass_buffer, "S3cureP@ssw0rdF0rCharlie", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    printf("Verifying 'charlie' with correct password: %s\n", verify_password("charlie", pass_buffer) ? "SUCCESS" : "FAILURE");

    strncpy(pass_buffer, "wrongpass", sizeof(pass_buffer) - 1);
    pass_buffer[sizeof(pass_buffer) - 1] = '\0';
    printf("Verifying 'charlie' with incorrect password: %s\n", verify_password("charlie", pass_buffer) ? "SUCCESS" : "FAILURE");

    cleanup_database();
    return 0;
}