#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define ITERATIONS 210000
#define SALT_SIZE 16 // bytes
#define KEY_LENGTH 32 // bytes, 256 bits

// Helper to convert bytes to hex. The caller must free the returned string.
char* bytes_to_hex(const unsigned char* data, size_t len) {
    if (len == 0 || len > (size_t)-1 / 2) { // Check for empty input and potential overflow
        return NULL;
    }
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * @brief Hashes a password using PBKDF2 with HMAC-SHA256.
 *
 * @param username The username (for context, not used in hash).
 * @param password The password to hash.
 * @return A dynamically allocated string containing the salt and hash in hex format,
 *         separated by a colon. Returns NULL on failure. The caller MUST free this string.
 */
char* registerUser(const char* username, const char* password) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[KEY_LENGTH];
    char* salt_hex = NULL;
    char* hash_hex = NULL;
    char* result = NULL;
    size_t password_len = strlen(password);

    // Rule #7: Use a strong, cryptographically secure random number generator.
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating random salt: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return NULL;
    }

    // Rule #5: Use PBKDF2-HMAC-SHA-256.
    if (PKCS5_PBKDF2_HMAC(
            password, password_len,
            salt, sizeof(salt),
            ITERATIONS, EVP_sha256(),
            sizeof(hash), hash) != 1) {
        fprintf(stderr, "Error in PBKDF2 HMAC: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return NULL;
    }

    // Rule #6: Store the salt with the hash. Convert to hex for storage.
    salt_hex = bytes_to_hex(salt, sizeof(salt));
    if (!salt_hex) {
        fprintf(stderr, "Memory allocation failed for salt hex string.\n");
        goto cleanup;
    }
    hash_hex = bytes_to_hex(hash, sizeof(hash));
    if (!hash_hex) {
        fprintf(stderr, "Memory allocation failed for hash hex string.\n");
        goto cleanup;
    }
    
    // Combine into a single string: "salt_hex:hash_hex"
    size_t final_len = strlen(salt_hex) + 1 + strlen(hash_hex) + 1;
    result = (char*)malloc(final_len);
    if (!result) {
        fprintf(stderr, "Memory allocation failed for final hash string.\n");
        goto cleanup;
    }
    
    snprintf(result, final_len, "%s:%s", salt_hex, hash_hex);

cleanup:
    free(salt_hex);
    free(hash_hex);
    // Rule #16: Clear sensitive data from memory.
    // `hash` contains derived key material and should be cleared.
    #if defined(_WIN32)
        SecureZeroMemory(hash, sizeof(hash));
    #elif defined(__STDC_LIB_EXT1__)
        memset_s(hash, sizeof(hash), 0, sizeof(hash));
    #else
        // Volatile write to prevent compiler optimization
        volatile unsigned char * p = hash;
        for(size_t i=0; i < sizeof(hash); i++) {
            p[i] = 0;
        }
    #endif

    return result;
}

int main() {
    printf("Running registration simulation...\n");

    const char* test_users[][2] = {
        {"alice", "Password123!"},
        {"bob", "my$ecretP@ssw0rd"},
        {"charlie", "an0th3r_sTr0ng_p@ss"},
        {"david", "w3ak"},
        {"eve", "Eve's_Password_is_very_long_and_secure_hopefully"}
    };

    size_t num_users = sizeof(test_users) / sizeof(test_users[0]);

    for (size_t i = 0; i < num_users; ++i) {
        const char* username = test_users[i][0];
        const char* password = test_users[i][1];

        char* hashedPassword = registerUser(username, password);
        if (hashedPassword) {
            printf("Username: %s\n", username);
            printf("Password: [REDACTED]\n");
            printf("Stored Hash: %s\n", hashedPassword);
            printf("---\n");
            free(hashedPassword);
        } else {
            fprintf(stderr, "Failed to register user %s.\n", username);
        }
    }

    return 0;
}