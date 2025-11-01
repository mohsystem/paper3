#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// This code requires linking with OpenSSL: gcc your_file.c -o your_app -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#define MIN_USERNAME_LENGTH 4
#define MAX_USERNAME_LENGTH 64
#define MIN_PASSWORD_LENGTH 8
#define MAX_PASSWORD_LENGTH 128

#define ITERATIONS 600000
#define KEY_LENGTH 32 // 256 bits
#define SALT_LENGTH 16 // 128 bits

// Rule #10: implement a helper to prevent compiler optimization of memset
void secure_zero_memory(void *v, size_t n) {
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

// Helper to convert byte array to hex string
// Caller must free the returned string
char* bytes_to_hex(const unsigned char* data, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Helper to convert hex string to byte array
// Caller must free the returned buffer
unsigned char* hex_to_bytes(const char* hex, size_t* out_len) {
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) return NULL;
    *out_len = hex_len / 2;
    unsigned char* bytes = (unsigned char*)malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; ++i) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
    return bytes;
}

/**
 * Checks if the username and password are correct.
 * @param username The username from the URL.
 * @param password The password from the URL.
 * @return "true" if credentials are valid, "false" otherwise.
 */
const char* check_mod(const char* username, const char* password) {
    // Rule #1: Ensure all input is validated.
    if (!username || !password) return "false";
    size_t user_len = strlen(username);
    size_t pass_len = strlen(password);
    if (user_len < MIN_USERNAME_LENGTH || user_len > MAX_USERNAME_LENGTH) return "false";
    if (pass_len < MIN_PASSWORD_LENGTH || pass_len > MAX_PASSWORD_LENGTH) return "false";

    // Rule #7: Avoid hardcoding credentials. Fetch from a secure source.
    const char* stored_username = getenv("CORRECT_USERNAME");
    const char* stored_hash_hex = getenv("PASSWORD_HASH");
    const char* stored_salt_hex = getenv("PASSWORD_SALT");

    if (!stored_username || !stored_hash_hex || !stored_salt_hex) {
        return "false"; // Configuration error
    }

    if (strcmp(username, stored_username) != 0) {
        return "false";
    }

    size_t stored_salt_len = 0;
    unsigned char* stored_salt = hex_to_bytes(stored_salt_hex, &stored_salt_len);
    if (!stored_salt) return "false";

    size_t stored_hash_len = 0;
    unsigned char* stored_hash = hex_to_bytes(stored_hash_hex, &stored_hash_len);
    if (!stored_hash) {
        free(stored_salt);
        return "false";
    }

    unsigned char generated_hash[KEY_LENGTH];
    int result = PKCS5_PBKDF2_HMAC(
        password, (int)pass_len,
        stored_salt, (int)stored_salt_len,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        generated_hash
    );
    
    bool success = false;
    if (result == 1) {
        // Rule #10: Use constant-time comparison to prevent timing attacks.
        if (stored_hash_len == KEY_LENGTH &&
            CRYPTO_memcmp(stored_hash, generated_hash, KEY_LENGTH) == 0) {
            success = true;
        }
    }
    
    // Securely clear sensitive data from memory
    secure_zero_memory(generated_hash, KEY_LENGTH);
    free(stored_salt);
    free(stored_hash);

    return success ? "true" : "false";
}

int main() {
    printf("--- Running 5 Test Cases ---\n");

    // --- Test Setup ---
    const char* correct_username = "testuser";
    const char* correct_password = "S3cur3P@ssw0rd!";
    
    // Rule #9: Use a cryptographically secure random number generator.
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate salt.\n");
        return 1;
    }

    // Rule #5, #8: Use strong hashing with a unique salt per password.
    unsigned char hash[KEY_LENGTH];
    if (PKCS5_PBKDF2_HMAC(correct_password, (int)strlen(correct_password), salt, SALT_LENGTH, ITERATIONS, EVP_sha256(), KEY_LENGTH, hash) != 1) {
        fprintf(stderr, "Failed to hash password.\n");
        return 1;
    }
    
    char* salt_hex = bytes_to_hex(salt, SALT_LENGTH);
    char* hash_hex = bytes_to_hex(hash, KEY_LENGTH);
    if (!salt_hex || !hash_hex) {
        fprintf(stderr, "Memory allocation failed for hex conversion.\n");
        free(salt_hex);
        free(hash_hex);
        return 1;
    }

    // Store credentials in environment variables for the test
    setenv("CORRECT_USERNAME", correct_username, 1);
    setenv("PASSWORD_SALT", salt_hex, 1);
    setenv("PASSWORD_HASH", hash_hex, 1);

    // --- Test Cases ---
    // Test Case 1: Correct credentials
    const char* result1 = check_mod(correct_username, correct_password);
    printf("Test 1 (Correct credentials): %s (Expected: true)\n", result1);

    // Test Case 2: Incorrect password
    const char* result2 = check_mod(correct_username, "wrongpassword");
    printf("Test 2 (Incorrect password): %s (Expected: false)\n", result2);

    // Test Case 3: Incorrect username
    const char* result3 = check_mod("wronguser", correct_password);
    printf("Test 3 (Incorrect username): %s (Expected: false)\n", result3);

    // Test Case 4: Null password
    const char* result4 = check_mod(correct_username, NULL);
    printf("Test 4 (Null password): %s (Expected: false)\n", result4);

    // Test Case 5: Password too short
    const char* result5 = check_mod(correct_username, "short");
    printf("Test 5 (Short password): %s (Expected: false)\n", result5);

    free(salt_hex);
    free(hash_hex);
    
    return 0;
}