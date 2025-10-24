/*
 * NOTE: This code requires the OpenSSL library.
 * To compile, you need to link against OpenSSL libraries.
 * Example compilation command:
 * gcc -o Task122 Task122.c -lssl -lcrypto
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

#define SALT_SIZE 16
#define HASH_SIZE 32
#define ITERATIONS 65536
#define HEX_BUFFER_SIZE (HASH_SIZE * 2 + 1)
#define STORED_BUFFER_SIZE (SALT_SIZE * 2 + 1 + HASH_SIZE * 2 + 1)

// Helper to convert byte array to hex string
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

// Helper to convert hex string to byte array
int hex_to_bytes(const char* hex_str, unsigned char* bytes, size_t max_len) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0 || len / 2 > max_len) {
        return 0; // Invalid hex string or buffer too small
    }
    for (size_t i = 0; i < len / 2; ++i) {
        if (sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]) != 1) {
            return 0; // Conversion error
        }
    }
    return 1;
}

/**
 * Stores a user password securely by generating a salt and hashing the password using PBKDF2.
 * @param password The plaintext password to store.
 * @param stored_password_out A buffer to store the output string (salt:hash).
 * @param buffer_size The size of the output buffer.
 * @return 1 on success, 0 on failure.
 */
int signup(const char* password, char* stored_password_out, size_t buffer_size) {
    if (buffer_size < STORED_BUFFER_SIZE) {
        return 0; // Buffer too small
    }

    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating random salt.\n");
        return 0;
    }

    unsigned char hash[HASH_SIZE];
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash),
        hash
    );

    if (result != 1) {
        fprintf(stderr, "Error during password hashing.\n");
        return 0;
    }

    char salt_hex[SALT_SIZE * 2 + 1];
    char hash_hex[HASH_SIZE * 2 + 1];
    bytes_to_hex(salt, sizeof(salt), salt_hex);
    bytes_to_hex(hash, sizeof(hash), hash_hex);
    
    snprintf(stored_password_out, buffer_size, "%s:%s", salt_hex, hash_hex);
    
    return 1;
}

/**
 * Verifies a plaintext password against a stored salted hash.
 * @param password The plaintext password to verify.
 * @param stored_password The stored string containing the salt and hash.
 * @return 1 if the password is correct, 0 otherwise.
 */
int verifyPassword(const char* password, const char* stored_password) {
    char salt_hex[SALT_SIZE * 2 + 1];
    char stored_hash_hex[HASH_SIZE * 2 + 1];

    if (sscanf(stored_password, "%[0-9a-fA-F]:%[0-9a-fA-F]", salt_hex, stored_hash_hex) != 2) {
        return 0; // Invalid format
    }
    if (strlen(salt_hex) != SALT_SIZE * 2 || strlen(stored_hash_hex) != HASH_SIZE * 2) {
        return 0; // Invalid length
    }

    unsigned char salt[SALT_SIZE];
    unsigned char stored_hash[HASH_SIZE];
    if (!hex_to_bytes(salt_hex, salt, sizeof(salt)) || !hex_to_bytes(stored_hash_hex, stored_hash, sizeof(stored_hash))) {
        return 0; // Hex conversion failed
    }
    
    unsigned char test_hash[HASH_SIZE];
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(test_hash),
        test_hash
    );

    if (result != 1) {
        return 0; // Hashing failed
    }

    // Constant-time comparison
    return CRYPTO_memcmp(stored_hash, test_hash, sizeof(stored_hash)) == 0;
}

void run_test_case(int case_num, const char* description, int condition, const char* fail_message) {
    printf("\n[Test Case %d: %s]\n", case_num, description);
    if (!condition) {
        fprintf(stderr, "TEST FAILED: %s\n", fail_message);
    }
}

int main() {
    printf("--- C Password Hashing Tests ---\n");
    
    char stored_user1[STORED_BUFFER_SIZE];
    const char* user1_password = "password123";
    
    // Test Case 1
    printf("\n[Test Case 1: Signup with 'password123']\n");
    if (signup(user1_password, stored_user1, sizeof(stored_user1))) {
        printf("Stored format for '%s': %s\n", user1_password, stored_user1);
    } else {
        printf("TEST FAILED: Signup failed.\n");
        return 1;
    }

    // Test Case 2
    int is_correct = verifyPassword(user1_password, stored_user1);
    run_test_case(2, "Verify with correct password 'password123'", is_correct, "Correct password did not verify.");
    printf("Verification successful: %s\n", is_correct ? "true" : "false");

    // Test Case 3
    int is_incorrect = verifyPassword("wrongpassword", stored_user1);
    run_test_case(3, "Verify with incorrect password 'wrongpassword'", !is_incorrect, "Incorrect password was verified.");
    printf("Verification successful: %s\n", is_incorrect ? "true" : "false");

    // Test Case 4
    char stored_user2[STORED_BUFFER_SIZE];
    const char* user2_password = "anotherPassword";
    printf("\n[Test Case 4: Signup with a different password 'anotherPassword']\n");
    if (signup(user2_password, stored_user2, sizeof(stored_user2))) {
        printf("Stored format for '%s': %s\n", user2_password, stored_user2);
        int are_different = strcmp(stored_user1, stored_user2) != 0;
        printf("Hashes for different passwords are different: %s\n", are_different ? "true" : "false");
        if (!are_different) printf("TEST FAILED!\n");
    } else {
        printf("TEST FAILED: Signup failed.\n");
    }

    // Test Case 5
    char stored_user1_again[STORED_BUFFER_SIZE];
    printf("\n[Test Case 5: Signup with 'password123' again]\n");
    if (signup(user1_password, stored_user1_again, sizeof(stored_user1_again))) {
        printf("Second stored format for '%s': %s\n", user1_password, stored_user1_again);
        int are_different_due_to_salt = strcmp(stored_user1, stored_user1_again) != 0;
        printf("Hashes for the same password are different due to salt: %s\n", are_different_due_to_salt ? "true" : "false");
        if (!are_different_due_to_salt) printf("TEST FAILED!\n");
    } else {
        printf("TEST FAILED: Signup failed.\n");
    }

    return 0;
}