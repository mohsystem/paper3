#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>

// Note: This code requires linking with OpenSSL's libcrypto.
// Example compilation: gcc your_file.c -o your_app -lcrypto

#define ITERATIONS 100000
#define KEY_LENGTH 32 // In bytes (256 bits)

/**
 * Converts a byte array into a hexadecimal string.
 * The caller is responsible for freeing the returned string.
 */
char* toHexString(const unsigned char* data, int len) {
    // Each byte is 2 hex chars + 1 for null terminator
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) {
        return NULL;
    }
    for (int i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", data[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

/**
 * Computes the hash of a password combined with a salt using PBKDF2.
 * The caller is responsible for freeing the returned string.
 */
char* computeHash(const char* password, const char* salt) {
    unsigned char* hash = (unsigned char*)malloc(KEY_LENGTH);
    if (!hash) {
        return NULL;
    }

    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        (const unsigned char*)salt,
        strlen(salt),
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash
    );

    if (result == 1) {
        char* hexHash = toHexString(hash, KEY_LENGTH);
        free(hash); // Clean up the raw hash buffer
        return hexHash;
    } else {
        free(hash);
        return NULL; // Return NULL on failure
    }
}

int main() {
    struct TestCase {
        const char* password;
        const char* salt;
    };

    struct TestCase testCases[] = {
        {"password123", "somesalt1"},
        {"P@$$w0rd!", "anothersalt2"},
        {"a_very_secure_password", "salty-salt-salt"},
        {"", "emptypassword"},
        {"test", ""}
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("C PBKDF2-HMAC-SHA256 Hashes:\n");
    for (int i = 0; i < numTestCases; i++) {
        char* hash = computeHash(testCases[i].password, testCases[i].salt);
        printf("Password: \"%s\", Salt: \"%s\"\n", testCases[i].password, testCases[i].salt);
        if (hash) {
            printf("Hash: %s\n", hash);
            free(hash); // Free the memory allocated by computeHash
        } else {
            printf("Hash computation failed.\n");
        }
        printf("\n");
    }
    
    return 0;
}