/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * gcc your_file_name.c -o Lse92 -lssl -lcrypto
 *
 * This code demonstrates secure password hashing using PBKDF2 from OpenSSL.
 * For simplicity, this example does not include a user database. It just
 * performs and prints the hashing result.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

// Helper function to print a byte array as a hex string
void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Registers a new user by securely hashing their password.
 * @param username The username to register.
 * @param password The user's password.
 * @return 0 on success, -1 on failure.
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        fprintf(stderr, "Registration failed: Username and password cannot be empty.\n");
        return -1;
    }

    // Generate a random salt (16 bytes is a good size)
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating salt.\n");
        return -1;
    }

    // Use PBKDF2 for hashing
    unsigned char hashedPassword[32]; // For SHA-256, 32 bytes output
    int iterations = 100000;
    
    if (PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt, 
        sizeof(salt),
        iterations,
        EVP_sha256(),
        sizeof(hashedPassword),
        hashedPassword
    ) == 0) {
        fprintf(stderr, "Error in PKCS5_PBKDF2_HMAC.\n");
        return -1;
    }

    printf("User '%s' registered successfully.\n", username);
    print_hex("  - Salt (Hex): ", salt, sizeof(salt));
    print_hex("  - Hashed Password (Hex): ", hashedPassword, sizeof(hashedPassword));
    printf("------------------------------------\n");

    return 0;
}

int main() {
    printf("--- C Registration Test Cases ---\n");
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "aVeryLongAndComplexPassword");
    registerUser("dave", "short");
    registerUser("frank", NULL); // Test null password
    
    return 0;
}