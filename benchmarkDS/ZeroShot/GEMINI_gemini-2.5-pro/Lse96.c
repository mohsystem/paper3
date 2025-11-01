#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

// This code requires the OpenSSL library.
// To compile on Linux/macOS: gcc -o your_program_name your_source_file.c -lssl -lcrypto
// You must have OpenSSL development libraries installed (e.g., `libssl-dev` on Debian/Ubuntu).

// Hashing parameters
#define SALT_LENGTH 16
#define ITERATIONS 65536
#define KEY_LENGTH 32 // Corresponds to 256 bits for SHA-256

/**
 * Generates a cryptographically secure random salt.
 * @param salt_buffer A buffer of at least SALT_LENGTH bytes to store the salt.
 * @return 1 on success, 0 on failure.
 */
int generateSalt(unsigned char* salt_buffer) {
    if (RAND_bytes(salt_buffer, SALT_LENGTH) != 1) {
        fprintf(stderr, "Error generating random salt.\n");
        return 0;
    }
    return 1;
}

/**
 * Hashes a password using PBKDF2-HMAC-SHA256.
 * @param password The password string.
 * @param salt The salt buffer.
 * @param key_buffer A buffer of at least KEY_LENGTH bytes to store the derived key.
 * @return 1 on success, 0 on failure.
 */
int hashPassword(const char* password, const unsigned char* salt, unsigned char* key_buffer) {
    if (password == NULL || strlen(password) == 0) {
        fprintf(stderr, "Password cannot be empty.\n");
        return 0;
    }
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LENGTH,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        key_buffer
    );
    if (result != 1) {
        fprintf(stderr, "Error in PKCS5_PBKDF2_HMAC.\n");
        return 0;
    }
    return 1;
}

/**
 * Converts a byte buffer to a hexadecimal string.
 * @param bytes The input byte buffer.
 * @param len The length of the input buffer.
 * @param hex_str The output buffer for the hex string (must be at least 2*len + 1 bytes).
 */
void bytesToHex(const unsigned char* bytes, int len, char* hex_str) {
    for (int i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

/**
 * Simulates user registration.
 * NOTE: The caller is responsible for freeing the returned string.
 * @param username The username string.
 * @param password The password string.
 * @return A dynamically allocated string containing "salt:hash", or NULL on failure.
 */
char* registerUser(const char* username, const char* password) {
    unsigned char salt[SALT_LENGTH];
    unsigned char hashedPassword[KEY_LENGTH];

    // 1. Generate salt
    if (!generateSalt(salt)) {
        return NULL;
    }

    // 2. Hash the password
    if (!hashPassword(password, salt, hashedPassword)) {
        return NULL;
    }

    // 3. Convert to hex and concatenate for storage
    char saltHex[SALT_LENGTH * 2 + 1];
    char hashHex[KEY_LENGTH * 2 + 1];
    bytesToHex(salt, SALT_LENGTH, saltHex);
    bytesToHex(hashedPassword, KEY_LENGTH, hashHex);

    // Allocate memory for the final string: "salt" + ":" + "hash" + "\0"
    size_t total_len = strlen(saltHex) + 1 + strlen(hashHex) + 1;
    char* storedCredentials = (char*)malloc(total_len);
    if (storedCredentials == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    snprintf(storedCredentials, total_len, "%s:%s", saltHex, hashHex);

    return storedCredentials;
}

void run_test_case(const char* username, const char* password) {
    char* storedCredentials = registerUser(username, password);
    if (storedCredentials) {
        printf("User: %s, Stored: %s\n", username, storedCredentials);
        free(storedCredentials); // IMPORTANT: Free the allocated memory
    } else {
        printf("Failed to register user: %s\n", username);
    }
}

// In C, a class isn't a native concept. We use a main function directly.
int main() {
    printf("--- Running C Registration Test Cases ---\n");
    
    // Test Case 1
    run_test_case("user1", "Password123!");

    // Test Case 2
    run_test_case("admin_test", "S3cureP@ssw0rd");

    // Test Case 3
    run_test_case("jane.doe", "another-password-4-jane");

    // Test Case 4: Simple password
    run_test_case("test_user", "password");

    // Test Case 5: Long password with many special characters
    run_test_case("poweruser", "L0ngP@$$w0rd_With_Many_Ch@r$!#%^&*()");

    return 0;
}