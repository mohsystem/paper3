#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

// Note: This code requires the OpenSSL library.
// Compile with: gcc your_file_name.c -o your_executable_name -lssl -lcrypto

/**
 * Hashes a password using the SHA-256 algorithm.
 * @param username The username (not used in hashing).
 * @param password The password to hash.
 * @return A dynamically allocated string containing the hex-encoded hash.
 *         The caller is responsible for freeing this memory. Returns NULL on failure.
 */
char* registerUser(const char* username, const char* password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    // Allocate memory for the hex string: 64 characters for the hash + 1 for the null terminator
    char* hexString = (char*)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (hexString == NULL) {
        fprintf(stderr, "Unable to allocate memory for hash string\n");
        return NULL;
    }

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hexString + (i * 2), "%02x", hash[i]);
    }
    hexString[SHA256_DIGEST_LENGTH * 2] = '\0'; // Null-terminate the string

    return hexString;
}

int main() {
    // 5 Test Cases
    const char* usernames[] = {"admin", "user", "guest", "test_user_long_name", "another_user"};
    const char* passwords[] = {"password123", "P@ssw0rd!", "", "a_very_long_and_secure_password_with_numbers_12345_and_symbols_!@#$", "password123"};
    int numTestCases = 5;
    
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* username = usernames[i];
        const char* password = passwords[i];
        char* hashedPassword = registerUser(username, password);
        
        printf("Username: %s\n", username);
        printf("Password: %s\n", password);
        if (hashedPassword != NULL) {
            printf("Hashed Password (SHA-256): %s\n", hashedPassword);
            free(hashedPassword); // Free the allocated memory
        } else {
            printf("Failed to hash password.\n");
        }
        printf("---------------------------------\n");
    }
    
    return 0;
}