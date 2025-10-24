#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

// Note: This code requires the OpenSSL library.
// To compile, you might need to link against it, for example:
// gcc your_file.c -o your_program -lssl -lcrypto

/**
 * Computes the SHA-256 hash of a password combined with a salt.
 *
 * @param password The password to hash.
 * @param salt The salt to combine with the password.
 * @return A dynamically allocated string containing the hexadecimal
 *         representation of the hash. The caller is responsible for
 *         freeing this memory.
 */
char* hashPassword(const char* password, const char* salt) {
    // Combine salt and password
    size_t pass_len = strlen(password);
    size_t salt_len = strlen(salt);
    char* combined = (char*)malloc(salt_len + pass_len + 1);
    if (!combined) {
        return NULL;
    }
    strcpy(combined, salt);
    strcat(combined, password);

    // Hash the combined string
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined, strlen(combined));
    SHA256_Final(hash, &sha256);
    
    free(combined); // Free the combined string memory

    // Convert hash to hex string
    char* hex_string = (char*)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (!hex_string) {
        return NULL;
    }
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_string + (i * 2), "%02x", hash[i]);
    }
    hex_string[SHA256_DIGEST_LENGTH * 2] = '\0';
    
    return hex_string;
}

int main() {
    // Test Case 1
    const char* pass1 = "password123";
    const char* salt1 = "randomsalt1";
    char* hash1 = hashPassword(pass1, salt1);
    printf("Password: %s, Salt: %s\n", pass1, salt1);
    printf("Hash: %s\n\n", hash1);
    free(hash1);

    // Test Case 2
    const char* pass2 = "MyP@ssw0rd";
    const char* salt2 = "salty-salty";
    char* hash2 = hashPassword(pass2, salt2);
    printf("Password: %s, Salt: %s\n", pass2, salt2);
    printf("Hash: %s\n\n", hash2);
    free(hash2);

    // Test Case 3
    const char* pass3 = "topsecret";
    const char* salt3 = "another-salt";
    char* hash3 = hashPassword(pass3, salt3);
    printf("Password: %s, Salt: %s\n", pass3, salt3);
    printf("Hash: %s\n\n", hash3);
    free(hash3);

    // Test Case 4 (Empty Password)
    const char* pass4 = "";
    const char* salt4 = "emptypass";
    char* hash4 = hashPassword(pass4, salt4);
    printf("Password: '%s', Salt: %s\n", pass4, salt4);
    printf("Hash: %s\n\n", hash4);
    free(hash4);

    // Test Case 5 (Empty Salt)
    const char* pass5 = "short";
    const char* salt5 = "";
    char* hash5 = hashPassword(pass5, salt5);
    printf("Password: %s, Salt: '%s'\n", pass5, salt5);
    printf("Hash: %s\n\n", hash5);
    free(hash5);

    return 0;
}