#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Constants for password hashing
#define ITERATIONS 310000
#define KEY_LEN_BYTES 32
#define SALT_LEN_BYTES 16
#define MAX_USERS 10

// User structure
typedef struct {
    char* username;
    unsigned char* stored_value;
    size_t stored_value_len;
} User;

// In-memory "database"
static User user_database[MAX_USERS];
static int user_count = 0;

void cleanup_database() {
    for (int i = 0; i < user_count; ++i) {
        free(user_database[i].username);
        // Securely clear and free the stored hash and salt
        if (user_database[i].stored_value) {
            OPENSSL_cleanse(user_database[i].stored_value, user_database[i].stored_value_len);
            free(user_database[i].stored_value);
        }
    }
}

// Helper to find a user
User* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            return &user_database[i];
        }
    }
    return NULL;
}

bool registerUser(const char* username, const char* password) {
    if (!username || username[0] == '\0' || !password || password[0] == '\0') {
        fprintf(stderr, "Username and password cannot be empty.\n");
        return false;
    }

    if (user_count >= MAX_USERS) {
        fprintf(stderr, "Database is full. Cannot register new user.\n");
        return false;
    }
    if (find_user(username) != NULL) {
        printf("User '%s' already exists.\n", username);
        return false;
    }

    unsigned char salt[SALT_LEN_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate random salt.\n");
        return false;
    }

    unsigned char hash[KEY_LEN_BYTES];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), ITERATIONS, EVP_sha256(), sizeof(hash), hash) != 1) {
        fprintf(stderr, "Failed to hash password.\n");
        return false;
    }
    
    // The stored value is salt + hash
    size_t combined_len = sizeof(salt) + sizeof(hash);
    unsigned char* combined = malloc(combined_len);
    if (!combined) {
        fprintf(stderr, "Memory allocation failed.\n");
        return false;
    }

    memcpy(combined, salt, sizeof(salt));
    memcpy(combined + sizeof(salt), hash, sizeof(hash));
    
    // Store user
    user_database[user_count].username = strdup(username);
    if (!user_database[user_count].username) {
        free(combined);
        fprintf(stderr, "Memory allocation failed for username.\n");
        return false;
    }
    user_database[user_count].stored_value = combined;
    user_database[user_count].stored_value_len = combined_len;
    user_count++;
    
    printf("User '%s' registered successfully.\n", username);
    return true;
}

bool loginUser(const char* username, const char* password) {
    if (!username || username[0] == '\0' || !password || password[0] == '\0') {
        return false;
    }

    User* user = find_user(username);
    if (user == NULL) {
        printf("Login failed: User '%s' not found.\n", username);
        return false;
    }

    if (user->stored_value_len != SALT_LEN_BYTES + KEY_LEN_BYTES) {
        fprintf(stderr, "Login failed: Invalid stored hash format for user '%s'.\n", username);
        return false;
    }

    // Extract salt and stored hash
    unsigned char* salt = user->stored_value;
    unsigned char* stored_hash = user->stored_value + SALT_LEN_BYTES;
    
    // Calculate hash of provided password with the stored salt
    unsigned char calculated_hash[KEY_LEN_BYTES];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LEN_BYTES, ITERATIONS, EVP_sha256(), sizeof(calculated_hash), calculated_hash) != 1) {
        fprintf(stderr, "Failed to hash password during login.\n");
        return false;
    }

    // Constant-time comparison
    bool match = (CRYPTO_memcmp(stored_hash, calculated_hash, KEY_LEN_BYTES) == 0);
    
    // Securely clear the calculated hash from memory
    OPENSSL_cleanse(calculated_hash, sizeof(calculated_hash));

    if (match) {
        printf("Login successful for user '%s'.\n", username);
    } else {
        printf("Login failed: Invalid password for user '%s'.\n", username);
    }

    return match;
}

int main() {
    printf("--- Running Test Cases ---\n");
    atexit(cleanup_database);

    // Test Case 1: Successful registration
    printf("\n[Test 1] Registering a new user 'alice'.\n");
    registerUser("alice", "P@ssw0rd123!");

    // Test Case 2: Attempting to register a user that already exists
    printf("\n[Test 2] Attempting to re-register 'alice'.\n");
    registerUser("alice", "anotherpassword");

    // Test Case 3: Successful login
    printf("\n[Test 3] Alice logs in with the correct password.\n");
    loginUser("alice", "P@ssw0rd123!");

    // Test Case 4: Failed login (wrong password)
    printf("\n[Test 4] Alice attempts to log in with an incorrect password.\n");
    loginUser("alice", "wrongpassword");

    // Test Case 5: Failed login (user not found)
    printf("\n[Test 5] Attempting to log in as a non-existent user 'bob'.\n");
    loginUser("bob", "anypassword");

    printf("\n--- Test Cases Finished ---\n");
    
    return 0;
}