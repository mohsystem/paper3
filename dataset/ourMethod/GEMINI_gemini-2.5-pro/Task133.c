#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <regex.h>

// To compile: gcc your_file.c -o your_app -lssl -lcrypto

// Security constants
#define SALT_LENGTH 16
#define TOKEN_LENGTH_BYTES 32
#define PBKDF2_ITERATIONS 310000
#define HASH_LENGTH_BYTES 32 // 256 bits for SHA256
#define TOKEN_VALIDITY_SECONDS 300 // 5 minutes
#define MAX_USERNAME_LEN 50
#define MAX_USERS 10
#define MAX_TOKENS 10

// Structs for in-memory storage (simulation)
typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char hashed_password[HASH_LENGTH_BYTES];
    unsigned char salt[SALT_LENGTH];
} User;

typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char hashed_token[HASH_LENGTH_BYTES];
    unsigned char salt[SALT_LENGTH];
    time_t expiration;
} ResetToken;

// Global in-memory storage for demonstration
static User user_database[MAX_USERS];
static int user_count = 0;
static ResetToken token_store[MAX_TOKENS];
static int token_count = 0;

void to_hex(char *dest, const unsigned char *src, size_t len) {
    for(size_t i = 0; i < len; ++i) {
        sprintf(dest + (i * 2), "%02x", src[i]);
    }
}

int hash_data(const char* data, const unsigned char* salt, unsigned char* hash_out) {
    return PKCS5_PBKDF2_HMAC(
        data,
        strlen(data),
        salt,
        SALT_LENGTH,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_LENGTH_BYTES,
        hash_out
    ) == 1;
}

bool is_password_strong(const char* password) {
    if (password == NULL || strlen(password) < 12) {
        return false;
    }
    regex_t regex;
    int ret;
    // Requires at least one digit, one lowercase, one uppercase, one special character
    const char *pattern = "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[!@#&()–[\\]{}:;',?/*~$^+=<>]).{12,}$";
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
        return false; 
    }
    ret = regexec(&regex, password, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

User* find_user(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            return &user_database[i];
        }
    }
    return NULL;
}

bool create_user(const char* username, const char* password) {
    if (username == NULL || strlen(username) >= MAX_USERNAME_LEN || user_count >= MAX_USERS || find_user(username) != NULL) {
        return false;
    }
    if (!is_password_strong(password)) return false;
    
    User* new_user = &user_database[user_count];
    strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
    new_user->username[MAX_USERNAME_LEN - 1] = '\0';

    if (RAND_bytes(new_user->salt, SALT_LENGTH) != 1) return false;
    if (!hash_data(password, new_user->salt, new_user->hashed_password)) return false;

    user_count++;
    return true;
}

char* request_password_reset(const char* username) {
    if (username == NULL || find_user(username) == NULL) return NULL;
    if (token_count >= MAX_TOKENS) return NULL;

    unsigned char token_bytes[TOKEN_LENGTH_BYTES];
    if (RAND_bytes(token_bytes, TOKEN_LENGTH_BYTES) != 1) return NULL;
    
    char* plain_text_token = malloc(TOKEN_LENGTH_BYTES * 2 + 1);
    if (!plain_text_token) return NULL;
    to_hex(plain_text_token, token_bytes, TOKEN_LENGTH_BYTES);
    plain_text_token[TOKEN_LENGTH_BYTES * 2] = '\0';
    
    ResetToken* new_token = &token_store[token_count++];
    strncpy(new_token->username, username, MAX_USERNAME_LEN - 1);
    new_token->username[MAX_USERNAME_LEN - 1] = '\0';

    if (RAND_bytes(new_token->salt, SALT_LENGTH) != 1) { free(plain_text_token); return NULL; }
    if (!hash_data(plain_text_token, new_token->salt, new_token->hashed_token)) { free(plain_text_token); return NULL; }
    
    new_token->expiration = time(NULL) + TOKEN_VALIDITY_SECONDS;
    return plain_text_token;
}

bool reset_password(const char* username, const char* token, const char* new_password) {
    if (!username || !token || !new_password || !find_user(username)) return false;

    ResetToken* stored_token = NULL;
    int token_idx = -1;
    for (int i = 0; i < token_count; ++i) {
        if (strcmp(token_store[i].username, username) == 0) {
            stored_token = &token_store[i];
            token_idx = i;
            break;
        }
    }
    if (!stored_token) return false;

    if (time(NULL) > stored_token->expiration) {
        // Invalidate by removing from store
        if (token_idx != -1) token_store[token_idx] = token_store[--token_count];
        return false;
    }

    unsigned char provided_token_hash[HASH_LENGTH_BYTES];
    if (!hash_data(token, stored_token->salt, provided_token_hash)) return false;
    
    // Constant-time comparison
    if (CRYPTO_memcmp(provided_token_hash, stored_token->hashed_token, HASH_LENGTH_BYTES) != 0) return false;
    
    if (!is_password_strong(new_password)) return false;

    User* user = find_user(username);
    if (!user) return false; // Should not happen but good to check

    unsigned char new_salt[SALT_LENGTH];
    if (RAND_bytes(new_salt, SALT_LENGTH) != 1) return false;
    memcpy(user->salt, new_salt, SALT_LENGTH);
    if (!hash_data(new_password, user->salt, user->hashed_password)) return false;

    // Invalidate token
    if (token_idx != -1) token_store[token_idx] = token_store[--token_count];

    return true;
}


int main() {
    printf("Running password reset tests...\n");
    const char* username = "testuser";
    const char* initial_password = "Password!12345";
    const char* new_password = "NewPassword@54321";

    if (create_user(username, initial_password)) {
        printf("1. User '%s' created successfully.\n", username);
    } else {
        fprintf(stderr, "1. Failed to create user.\n");
        return 1;
    }

    printf("\n--- Test Case 1: Successful Reset ---\n");
    char* token1 = request_password_reset(username);
    if (token1) {
        printf("Generated reset token: %s\n", token1);
        bool success1 = reset_password(username, token1, new_password);
        printf("Reset result: %s\n", success1 ? "SUCCESS" : "FAILURE");
        if (!success1) fprintf(stderr, "Expected SUCCESS\n");
    } else {
        fprintf(stderr, "Token generation failed.\n");
    }

    printf("\n--- Test Case 2: Reuse Token (Should Fail) ---\n");
    bool success2 = reset_password(username, token1, "AnotherPassword$99");
    printf("Reset result: %s\n", success2 ? "SUCCESS" : "FAILURE");
    if (success2) fprintf(stderr, "Expected FAILURE\n");
    free(token1);

    printf("\n--- Test Case 3: Wrong Token (Should Fail) ---\n");
    char* token3 = request_password_reset(username);
    if (token3) {
        printf("Generated new reset token: %s\n", token3);
        bool success3 = reset_password(username, "wrongTokenAbc123", new_password);
        printf("Reset result: %s\n", success3 ? "SUCCESS" : "FAILURE");
        if (success3) fprintf(stderr, "Expected FAILURE\n");
        free(token3);
    }

    printf("\n--- Test Case 4: Weak New Password (Should Fail) ---\n");
    char* token4 = request_password_reset(username);
    if (token4) {
        bool success4 = reset_password(username, token4, "weak");
        printf("Reset result for weak password: %s\n", success4 ? "SUCCESS" : "FAILURE");
        if (success4) fprintf(stderr, "Expected FAILURE\n");
        free(token4);
    }

    printf("\n--- Test Case 5: Expired Token (Should Fail) ---\n");
    printf("Simulating expired token test. In a real scenario, this would fail after %d seconds.\n", TOKEN_VALIDITY_SECONDS);

    return 0;
}