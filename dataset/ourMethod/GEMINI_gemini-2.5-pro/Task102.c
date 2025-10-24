#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>

// Note: This code requires linking with OpenSSL libraries, e.g., gcc ... -lssl -lcrypto

// Configuration for PBKDF2
#define ITERATIONS 210000
#define KEY_LENGTH 32 // 256 bits
#define SALT_SIZE 16  // 128 bits

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_HASH_LEN 256

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[MAX_HASH_LEN];
} User;

// In-memory array to simulate a database
User user_database[MAX_USERS];
int user_count = 0;

// Helper to convert bytes to a hex string. Caller must free the returned string.
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Helper to convert hex string to bytes. Caller must free the returned buffer.
unsigned char* hex_to_bytes(const char* hex, size_t* out_len) {
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) return NULL;
    *out_len = hex_len / 2;
    unsigned char* bytes = (unsigned char*)malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; ++i) {
        if (sscanf(hex + 2 * i, "%2hhx", &bytes[i]) != 1) {
            free(bytes);
            return NULL;
        }
    }
    return bytes;
}

// Caller must free the returned hash string
char* hash_password(const char* password) {
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate random salt.\n");
        return NULL;
    }

    unsigned char hash[KEY_LENGTH];
    int result = PKCS5_PBKDF2_HMAC(
        password, (int)strlen(password),
        salt, sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash), hash
    );

    if (result != 1) {
        fprintf(stderr, "Failed to execute PBKDF2.\n");
        return NULL;
    }

    char* salt_hex = bytes_to_hex(salt, sizeof(salt));
    char* hash_hex = bytes_to_hex(hash, sizeof(hash));
    if (!salt_hex || !hash_hex) {
        free(salt_hex);
        free(hash_hex);
        return NULL;
    }

    // Format as "salt:hash"
    size_t total_len = strlen(salt_hex) + strlen(hash_hex) + 2;
    char* combined_hash = (char*)malloc(total_len);
    if (combined_hash) {
        snprintf(combined_hash, total_len, "%s:%s", salt_hex, hash_hex);
    }
    
    free(salt_hex);
    free(hash_hex);
    return combined_hash;
}

bool verify_password(const char* password, const char* stored_hash) {
    const char* colon = strchr(stored_hash, ':');
    if (!colon) return false;

    size_t salt_hex_len = colon - stored_hash;
    char* salt_hex = (char*)malloc(salt_hex_len + 1);
    if (!salt_hex) return false;
    strncpy(salt_hex, stored_hash, salt_hex_len);
    salt_hex[salt_hex_len] = '\0';

    const char* hash_hex = colon + 1;
    
    size_t salt_len;
    unsigned char* salt = hex_to_bytes(salt_hex, &salt_len);
    free(salt_hex);
    if (!salt) return false;

    size_t original_hash_len;
    unsigned char* original_hash = hex_to_bytes(hash_hex, &original_hash_len);
    if (!original_hash) {
        free(salt);
        return false;
    }

    bool success = false;
    if (salt_len == SALT_SIZE && original_hash_len == KEY_LENGTH) {
        unsigned char test_hash[KEY_LENGTH];
        if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len, ITERATIONS, EVP_sha256(), sizeof(test_hash), test_hash) == 1) {
            // Constant-time comparison
            if (CRYPTO_memcmp(original_hash, test_hash, KEY_LENGTH) == 0) {
                success = true;
            }
        }
    }

    free(salt);
    free(original_hash);
    return success;
}


bool update_user_password(const char* username, const char* new_password) {
    if (!username || strlen(username) == 0 || strlen(username) >= MAX_USERNAME_LEN) {
        printf("Update failed: Invalid username.\n");
        return false;
    }
    if (!new_password || strlen(new_password) < 12) {
        printf("Update failed: Password must be at least 12 characters long.\n");
        return false;
    }
    
    char* new_hashed_password = hash_password(new_password);
    if (!new_hashed_password) {
        printf("Failed to hash the new password for user '%s'.\n", username);
        return false;
    }

    // Find user or create a new one
    int user_idx = -1;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            user_idx = i;
            break;
        }
    }
    
    if (user_idx == -1) { // New user
        if (user_count >= MAX_USERS) {
            printf("Database is full. Cannot add new user.\n");
            free(new_hashed_password);
            return false;
        }
        user_idx = user_count++;
        strncpy(user_database[user_idx].username, username, MAX_USERNAME_LEN - 1);
        user_database[user_idx].username[MAX_USERNAME_LEN - 1] = '\0';
    }

    strncpy(user_database[user_idx].password_hash, new_hashed_password, MAX_HASH_LEN - 1);
    user_database[user_idx].password_hash[MAX_HASH_LEN - 1] = '\0';
    
    printf("Password for user '%s' has been updated successfully.\n", username);
    free(new_hashed_password);
    return true;
}

const char* get_user_hash(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            return user_database[i].password_hash;
        }
    }
    return NULL;
}

int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Create a new user with a strong password
    printf("\n[Test Case 1: Create new user 'alice']\n");
    update_user_password("alice", "Str0ngP@ssw0rd123!");
    
    // Test Case 2: Verify the correct password
    printf("\n[Test Case 2: Verify correct password for 'alice']\n");
    const char* alice_hash = get_user_hash("alice");
    if(alice_hash){
        bool is_verified = verify_password("Str0ngP@ssw0rd123!", alice_hash);
        printf("Verification result: %s\n", is_verified ? "Success" : "Failure");
    }

    // Test Case 3: Verify an incorrect password
    printf("\n[Test Case 3: Verify incorrect password for 'alice']\n");
    if(alice_hash){
        bool is_verified = verify_password("WrongPassword!", alice_hash);
        printf("Verification result: %s\n", is_verified ? "Success" : "Failure");
    }
    
    // Test Case 4: Update the password for an existing user
    printf("\n[Test Case 4: Update password for 'alice']\n");
    update_user_password("alice", "EvenM0reSecur3P@ss!");
    
    // Test Case 5: Verify the new password
    printf("\n[Test Case 5: Verify the new password for 'alice']\n");
    const char* new_alice_hash = get_user_hash("alice");
    if(new_alice_hash){
        bool is_verified = verify_password("EvenM0reSecur3P@ss!", new_alice_hash);
        printf("Verification result: %s\n", is_verified ? "Success" : "Failure");
    }

    printf("\n--- Additional Tests ---\n");
    // Test Case 6: Attempt to update password with a weak password
    printf("\n[Test Case 6: Attempt weak password for 'bob']\n");
    update_user_password("bob", "weak");

    // Test Case 7: Attempt to update password with an empty username
    printf("\n[Test Case 7: Attempt update with empty username]\n");
    update_user_password("", "ValidPassword12345");

    return 0;
}