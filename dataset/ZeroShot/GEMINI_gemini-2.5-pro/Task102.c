/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you must link against OpenSSL libraries.
 * Example compilation command on Linux/macOS:
 * gcc your_file_name.c -o your_program_name -lssl -lcrypto
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define MAX_USERS 10
#define USERNAME_LEN 50
#define STORED_HASH_LEN 257 // 16-byte salt (32 hex) + ':' + 64-byte hash (128 hex) + null terminator = 161. 257 is safe.

#define SALT_SIZE 16
#define HASH_SIZE 64 // For SHA-512
#define ITERATIONS 65536

// Simulate a database with a simple struct and array
typedef struct {
    char username[USERNAME_LEN];
    char stored_hash[STORED_HASH_LEN];
} User;

User user_database[MAX_USERS];
int user_count = 0;

// Helper to convert byte array to hex string
void bytes_to_hex(const unsigned char* input, size_t len, char* output) {
    for (size_t i = 0; i < len; i++) {
        sprintf(output + (i * 2), "%02x", input[i]);
    }
}

// Hashes password and returns a newly allocated string "salt_hex:hash_hex"
// The caller is responsible for freeing the returned string.
char* hash_password(const char* password) {
    unsigned char salt[SALT_SIZE];
    if (!RAND_bytes(salt, sizeof(salt))) {
        fprintf(stderr, "Error generating salt.\n");
        return NULL;
    }

    unsigned char hash[HASH_SIZE];
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha512(),
        sizeof(hash),
        hash
    );
    
    if (result != 1) {
        fprintf(stderr, "Error in PKCS5_PBKDF2_HMAC.\n");
        return NULL;
    }
    
    // Allocate memory for the final "salt:hash" string
    // 2*SALT_SIZE for salt hex, 2*HASH_SIZE for hash hex, 1 for ':', 1 for null terminator
    char* stored_value = (char*)malloc(2 * SALT_SIZE + 2 * HASH_SIZE + 2);
    if (!stored_value) {
        perror("Failed to allocate memory");
        return NULL;
    }
    
    char salt_hex[2 * SALT_SIZE + 1];
    char hash_hex[2 * HASH_SIZE + 1];

    bytes_to_hex(salt, SALT_SIZE, salt_hex);
    bytes_to_hex(hash, HASH_SIZE, hash_hex);
    
    sprintf(stored_value, "%s:%s", salt_hex, hash_hex);
    
    return stored_value;
}

// Accepts username and new password, hashes the password, and updates the database.
int update_user_password(const char* username, const char* new_password) {
    if (username == NULL || *username == '\0' || new_password == NULL || *new_password == '\0') {
        fprintf(stderr, "Error: Username and password cannot be empty.\n");
        return 0; // Failure
    }
    
    int user_index = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            user_index = i;
            break;
        }
    }
    
    if (user_index == -1) {
        fprintf(stderr, "Error: User '%s' not found.\n", username);
        return 0; // Failure
    }
    
    char* new_hash = hash_password(new_password);
    if (new_hash == NULL) {
        fprintf(stderr, "Failed to hash password for user '%s'.\n", username);
        return 0; // Failure
    }
    
    strncpy(user_database[user_index].stored_hash, new_hash, STORED_HASH_LEN - 1);
    user_database[user_index].stored_hash[STORED_HASH_LEN - 1] = '\0';
    
    free(new_hash); // Free the memory allocated by hash_password
    
    printf("Password for user '%s' has been updated successfully.\n", username);
    return 1; // Success
}

void add_user(const char* username, const char* initial_hash) {
    if (user_count < MAX_USERS) {
        strncpy(user_database[user_count].username, username, USERNAME_LEN - 1);
        user_database[user_count].username[USERNAME_LEN-1] = '\0';
        strncpy(user_database[user_count].stored_hash, initial_hash, STORED_HASH_LEN - 1);
        user_database[user_count].stored_hash[STORED_HASH_LEN - 1] = '\0';
        user_count++;
    }
}

void print_database() {
    for (int i = 0; i < user_count; i++) {
        printf("User: %s, Stored: %s\n", user_database[i].username, user_database[i].stored_hash);
    }
}

int main() {
    // Initialize the 'database'
    add_user("alice", "initial_placeholder_hash");
    add_user("bob", "initial_placeholder_hash");
    add_user("charlie", "initial_placeholder_hash");
    add_user("david", "initial_placeholder_hash");
    add_user("eve", "initial_placeholder_hash");
    
    printf("--- Initial Database State ---\n");
    print_database();
    
    printf("\n--- Running 5 Test Cases for Password Update ---\n");
    
    // Test Case 1: Update alice's password
    update_user_password("alice", "P@ssword123!");
    
    // Test Case 2: Update bob's password
    update_user_password("bob", "SecurePwd!#$987");
    
    // Test Case 3: Update a non-existent user
    update_user_password("frank", "somePassword");
    
    // Test Case 4: Update charlie's password
    update_user_password("charlie", "another-strong-password-4-charlie");
    
    // Test Case 5: Update with an empty password
    update_user_password("david", "");
    
    printf("\n--- Final Database State ---\n");
    print_database();
    
    return 0;
}