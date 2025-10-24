/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against it. Example command:
 * gcc -o your_program_name Task122.c -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

#define SALT_LENGTH 16
#define KEY_LENGTH 32 // 256 bits
#define ITERATION_COUNT 65536
#define MAX_USERNAME_LEN 50
#define MAX_STORED_PASS_LEN (SALT_LENGTH * 2 + KEY_LENGTH * 2 + 2) // salt_hex:hash_hex\0
#define MAX_USERS 10

// --- Data Structure for User Storage ---
typedef struct {
    char username[MAX_USERNAME_LEN];
    char stored_password[MAX_STORED_PASS_LEN];
} User;

typedef struct {
    User users[MAX_USERS];
    int user_count;
} UserDatabase;

// --- Helper Functions ---
// Converts a byte array to a hex string. Caller must free the returned string.
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Converts a hex string to a byte array. Caller must free the returned buffer.
unsigned char* hex_to_bytes(const char* hex_str, size_t* out_len) {
    size_t hex_len = strlen(hex_str);
    if (hex_len % 2 != 0) return NULL;
    *out_len = hex_len / 2;
    unsigned char* bytes = (unsigned char*)malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; ++i) {
        sscanf(hex_str + i * 2, "%2hhx", &bytes[i]);
    }
    return bytes;
}

// --- Core Functions ---
void init_database(UserDatabase* db) {
    db->user_count = 0;
}

User* find_user(UserDatabase* db, const char* username) {
    for (int i = 0; i < db->user_count; ++i) {
        if (strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

int signup(UserDatabase* db, const char* username, const char* password) {
    if (db->user_count >= MAX_USERS) {
        fprintf(stderr, "Error: Database is full.\n");
        return 0;
    }
    if (find_user(db, username)) {
        fprintf(stderr, "Error: User '%s' already exists.\n", username);
        return 0;
    }

    // 1. Generate salt
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating random salt.\n");
        return 0;
    }

    // 2. Generate hash
    unsigned char hash[KEY_LENGTH];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt),
                         ITERATION_COUNT, EVP_sha256(), sizeof(hash), hash) != 1) {
        fprintf(stderr, "Error hashing password.\n");
        return 0;
    }

    // 3. Convert to hex and store
    char* salt_hex = bytes_to_hex(salt, sizeof(salt));
    char* hash_hex = bytes_to_hex(hash, sizeof(hash));
    if (!salt_hex || !hash_hex) {
        free(salt_hex);
        free(hash_hex);
        fprintf(stderr, "Memory allocation error.\n");
        return 0;
    }

    User* new_user = &db->users[db->user_count];
    strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
    new_user->username[MAX_USERNAME_LEN-1] = '\0';
    snprintf(new_user->stored_password, MAX_STORED_PASS_LEN, "%s:%s", salt_hex, hash_hex);
    
    free(salt_hex);
    free(hash_hex);
    db->user_count++;
    return 1;
}

int verify_password(UserDatabase* db, const char* username, const char* password) {
    User* user = find_user(db, username);
    if (!user) {
        return 0; // User not found
    }

    char stored_data[MAX_STORED_PASS_LEN];
    strcpy(stored_data, user->stored_password);

    char* salt_hex = strtok(stored_data, ":");
    char* stored_hash_hex = strtok(NULL, ":");

    if (!salt_hex || !stored_hash_hex) {
        return 0; // Invalid format
    }

    size_t salt_len, stored_hash_len;
    unsigned char* salt = hex_to_bytes(salt_hex, &salt_len);
    unsigned char* stored_hash = hex_to_bytes(stored_hash_hex, &stored_hash_len);

    if (!salt || !stored_hash) {
        free(salt);
        free(stored_hash);
        return 0;
    }

    unsigned char provided_hash[KEY_LENGTH];
    PKCS5_PBKDF2_HMAC(password, strlen(password), salt, salt_len,
                     ITERATION_COUNT, EVP_sha256(), sizeof(provided_hash), provided_hash);

    int match = 0;
    if (stored_hash_len == sizeof(provided_hash)) {
        if (CRYPTO_memcmp(stored_hash, provided_hash, stored_hash_len) == 0) {
            match = 1;
        }
    }

    free(salt);
    free(stored_hash);
    return match;
}

int main(void) {
    UserDatabase db;
    init_database(&db);

    printf("--- Test Cases ---\n");
    const char* true_str = "true";
    const char* false_str = "false";

    // Test Case 1: Successful signup
    printf("\n[Test 1] Successful Signup\n");
    int signup1 = signup(&db, "alice", "Password123!");
    printf("Signup for 'alice' successful: %s\n", signup1 ? true_str : false_str);

    // Test Case 2: Successful login
    printf("\n[Test 2] Successful Login\n");
    int login2 = verify_password(&db, "alice", "Password123!");
    printf("Login for 'alice' with correct password successful: %s\n", login2 ? true_str : false_str);

    // Test Case 3: Failed login (wrong password)
    printf("\n[Test 3] Failed Login (Wrong Password)\n");
    int login3 = verify_password(&db, "alice", "WrongPass!");
    printf("Login for 'alice' with incorrect password successful: %s\n", login3 ? true_str : false_str);

    // Test Case 4: Failed login (user does not exist)
    printf("\n[Test 4] Failed Login (User Not Found)\n");
    int login4 = verify_password(&db, "bob", "Password123!");
    printf("Login for 'bob' (non-existent user) successful: %s\n", login4 ? true_str : false_str);

    // Test Case 5: Failed signup (user already exists)
    printf("\n[Test 5] Failed Signup (User Exists)\n");
    signup(&db, "charlie", "MySecretPa$$");
    int signup5 = signup(&db, "charlie", "AnotherPassword");
    printf("Second signup for 'charlie' successful: %s\n", signup5 ? true_str : false_str);

    return 0;
}