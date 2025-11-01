#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// To compile: gcc your_file.c -o your_app -lssl -lcrypto

// Use a high iteration count for PBKDF2 as recommended by security standards.
#define ITERATIONS 210000
#define KEY_LENGTH_BYTES 32 // 256 bits
#define SALT_LENGTH_BYTES 16

// In a real application, this would be a proper database.
// This is a simplified demonstration.

/**
 * Converts a byte array to a hex string. Caller must free the returned string.
 */
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (hex_str == NULL) {
        perror("malloc failed");
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * Simulates inserting user data into a database.
 */
void insert_into_database(const char* username, const char* stored_password) {
    // In a real application, this would execute an SQL INSERT statement.
    printf("User '%s' registered successfully.\n", username);
    // For demonstration, we print the stored value. In production, never log this.
    printf("Stored format (salt:hash): %s\n", stored_password);
    printf("----------------------------------------\n");
}

/**
 * Registers a new user by hashing their password and storing the result.
 * Returns 0 on success, -1 on failure.
 */
int register_user(const char* username, const char* password) {
    if (username == NULL || *username == '\0' || password == NULL || *password == '\0') {
        fprintf(stderr, "Error: Username and password cannot be empty.\n");
        printf("----------------------------------------\n");
        return -1;
    }

    unsigned char salt[SALT_LENGTH_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error: Failed to generate salt.\n");
        return -1;
    }

    unsigned char hash[KEY_LENGTH_BYTES];
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
        fprintf(stderr, "Error: Failed to hash password. OpenSSL error: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }
    
    // Caller is responsible for clearing the password buffer if it's mutable.
    // If password were `char*`, we would use OPENSSL_cleanse here.

    char* salt_hex = bytes_to_hex(salt, sizeof(salt));
    char* hash_hex = bytes_to_hex(hash, sizeof(hash));

    if (salt_hex == NULL || hash_hex == NULL) {
        free(salt_hex);
        free(hash_hex);
        fprintf(stderr, "Error: Failed to convert binary to hex.\n");
        return -1;
    }

    // Allocate memory for the combined "salt:hash" string.
    size_t stored_len = strlen(salt_hex) + 1 + strlen(hash_hex) + 1;
    char* stored_password = (char*)malloc(stored_len);
    if (stored_password == NULL) {
        free(salt_hex);
        free(hash_hex);
        perror("malloc failed for stored_password");
        return -1;
    }

    snprintf(stored_password, stored_len, "%s:%s", salt_hex, hash_hex);

    insert_into_database(username, stored_password);

    // Clean up allocated memory
    free(salt_hex);
    free(hash_hex);
    free(stored_password);

    return 0;
}

int main() {
    printf("--- Running User Registration Test Cases ---\n");

    // Test Case 1
    register_user("alice", "Password123!");

    // Test Case 2
    register_user("bob", "S3cureP@ssw0rd");

    // Test Case 3
    register_user("charlie", "my-weak-password");
    
    // Test Case 4: Empty password
    register_user("david", "");

    // Test Case 5: Empty username
    register_user("", "anotherPassword");

    return 0;
}