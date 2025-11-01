#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// IMPORTANT: This code requires the OpenSSL library.
// To compile on Linux/macOS, you need to link against it:
// gcc your_file_name.c -o output_name -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

/**
 * Converts a byte array to a dynamically allocated hexadecimal string.
 * The caller is responsible for freeing the returned string.
 * @param data Pointer to the byte array.
 * @param len Length of the byte array.
 * @return A pointer to the newly allocated hexadecimal string.
 */
char* bytesToHex(const unsigned char* data, size_t len) {
    // Each byte becomes 2 hex characters, plus 1 for the null terminator.
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (hex_str == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * Registers a user by hashing their password and simulating a database insert.
 * @param username The username to register.
 * @param password The plaintext password.
 */
void registerUser(const char* username, const char* password) {
    printf("Registering user: %s\n", username);

    // 1. Generate a cryptographically secure random salt
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error: Could not generate random salt.\n");
        return;
    }

    // 2. Hash the password with the salt
    // Note: For production, use a key derivation function like PKCS5_PBKDF2_HMAC
    // from OpenSSL, which is more secure against brute-force attacks.
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, salt, sizeof(salt));
    SHA256_Update(&sha256_ctx, password, strlen(password));
    SHA256_Final(hash, &sha256_ctx);

    // Convert binary salt and hash to hex for storage
    char* saltHex = bytesToHex(salt, sizeof(salt));
    char* hashedPasswordHex = bytesToHex(hash, SHA256_DIGEST_LENGTH);

    if (saltHex == NULL || hashedPasswordHex == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for hex strings.\n");
        free(saltHex);
        free(hashedPasswordHex);
        return;
    }

    // 3. Simulate inserting into the database
    printf("  -> Simulating database insert...\n");
    printf("     Username: %s\n", username);
    printf("     Salt: %s\n", saltHex);
    printf("     Hashed Password: %s\n", hashedPasswordHex);
    printf("Registration complete.\n\n");

    // Clean up allocated memory
    free(saltHex);
    free(hashedPasswordHex);
}

int main() {
    // 5 Test Cases
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "qwerty");
    registerUser("diana", "12345678");
    registerUser("eve", "pa$$w0rd");

    return 0;
}