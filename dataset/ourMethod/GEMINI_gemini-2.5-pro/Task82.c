#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#define ITERATIONS 600000
#define KEY_LENGTH 32 // 32 bytes = 256 bits
#define SALT_LENGTH 16 // 16 bytes = 128 bits

/**
 * Converts a byte array to a hex string.
 * The caller must free the returned string.
 *
 * @param bytes The byte array.
 * @param len The length of the byte array.
 * @return A dynamically allocated hex-encoded string, or NULL on failure.
 */
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    if (bytes == NULL || len == 0) return NULL;
    size_t str_len = len * 2 + 1;
    char* hex_str = (char*)malloc(str_len);
    if (hex_str == NULL) return NULL;

    for (size_t i = 0; i < len; i++) {
        // Use snprintf for safety. The buffer size is 3 for "xx" and a null terminator.
        if (snprintf(hex_str + i * 2, 3, "%02x", bytes[i]) < 0) {
            free(hex_str);
            return NULL;
        }
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * Generates a cryptographically secure random salt.
 *
 * @param salt_out Buffer to store the generated salt (must be at least SALT_LENGTH).
 * @return 1 on success, 0 on failure.
 */
int generate_salt(unsigned char* salt_out) {
    if (salt_out == NULL) return 0;
    if (RAND_bytes(salt_out, SALT_LENGTH) != 1) {
        return 0;
    }
    return 1;
}

/**
 * Computes the hash of a password combined with a salt using PBKDF2 with HMAC-SHA256.
 * The caller must free the returned string.
 *
 * @param password The password to hash.
 * @param salt The salt to use.
 * @param salt_len The length of the salt.
 * @return A dynamically allocated hex-encoded string of the hash, or NULL on failure.
 */
char* hash_password(const char* password, const unsigned char* salt, size_t salt_len) {
    if (password == NULL || salt == NULL) {
        return NULL;
    }

    unsigned char* hash = (unsigned char*)malloc(KEY_LENGTH);
    if (hash == NULL) {
        return NULL;
    }

    int success = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        salt_len,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash
    );

    char* hex_hash = NULL;
    if (success == 1) {
        hex_hash = bytes_to_hex(hash, KEY_LENGTH);
    }
    
    free(hash);
    return hex_hash;
}

void run_test_case(const char* password) {
    // In a real application, never log or print the plaintext password.
    // This is for demonstration purposes only.
    printf("\n--- Test Case ---\n");
    printf("Password (for demo only): %s\n", password);

    unsigned char salt[SALT_LENGTH];
    if (!generate_salt(salt)) {
        fprintf(stderr, "Failed to generate salt.\n");
        return;
    }

    char* hex_salt = bytes_to_hex(salt, SALT_LENGTH);
    if (!hex_salt) {
        fprintf(stderr, "Failed to convert salt to hex.\n");
        return;
    }
    printf("Generated Salt (hex): %s\n", hex_salt);
    free(hex_salt);

    char* hashed_password = hash_password(password, salt, SALT_LENGTH);
    if (!hashed_password) {
        fprintf(stderr, "Failed to hash password.\n");
        return;
    }
    
    printf("Hashed Password (hex): %s\n", hashed_password);
    free(hashed_password);
}


int main(void) {
    const char* test_passwords[] = {
        "P@ssword123!",
        "correct horse battery staple",
        "Tr0ub4dor&3",
        "123456",
        "super-secret-password"
    };
    int num_passwords = sizeof(test_passwords) / sizeof(test_passwords[0]);

    printf("Running %d test cases for password hashing...\n", num_passwords);

    for (int i = 0; i < num_passwords; i++) {
        run_test_case(test_passwords[i]);
    }

    return 0;
}