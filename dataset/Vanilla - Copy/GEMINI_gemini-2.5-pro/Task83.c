// To compile and run this code:
// 1. Install OpenSSL development libraries (e.g., on Debian/Ubuntu: sudo apt-get install libssl-dev)
// 2. Compile with: gcc this_file.c -o task83_c -lssl -lcrypto
// 3. Run: ./task83_c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h> // For AES_BLOCK_SIZE

void handle_openssl_errors(const char* message) {
    fprintf(stderr, "ERROR: %s\n", message);
    ERR_print_errors_fp(stderr);
    // In a real application, you might want more graceful error handling
    exit(EXIT_FAILURE);
}

/**
 * @brief Encrypts plaintext using AES-128-CBC.
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the data to encrypt.
 * @param key The 16-byte encryption key.
 * @param iv The 16-byte initialization vector.
 * @param ciphertext A pointer to a buffer that will be allocated to hold the ciphertext.
 * @return The length of the ciphertext, or -1 on error.
 * @note The caller is responsible for freeing the memory allocated for *ciphertext.
 */
int encrypt(const unsigned char* plaintext, int plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char** ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len = 0;
    int ciphertext_len = 0;

    // Allocate buffer for ciphertext. PKCS padding can add up to a full block.
    *ciphertext = (unsigned char *)malloc(plaintext_len + AES_BLOCK_SIZE);
    if (!*ciphertext) {
        perror("malloc failed");
        return -1;
    }

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        free(*ciphertext);
        handle_openssl_errors("EVP_CIPHER_CTX_new failed");
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
        free(*ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors("EVP_EncryptInit_ex failed");
    }

    if (1 != EVP_EncryptUpdate(ctx, *ciphertext, &len, plaintext, plaintext_len)) {
        free(*ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors("EVP_EncryptUpdate failed");
    }
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, *ciphertext + len, &len)) {
        free(*ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    const unsigned char key[] = "0123456789abcdef";
    const unsigned char iv[] = "fedcba9876543210";

    const char* testCases[] = {
        "This is a test.",
        "Short",
        "This is a slightly longer plaintext message.",
        "Encrypt this key!",
        "AES CBC Mode Test"
    };
    const int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("AES-128-CBC Encryption Test Cases:\n");
    printf("Key: %s\n", key);
    printf("IV: %s\n", iv);
    printf("----------------------------------------------\n");

    for (int i = 0; i < numTestCases; i++) {
        const char* plainText = testCases[i];
        unsigned char* encrypted_text = NULL;
        
        int encrypted_len = encrypt((const unsigned char*)plainText, strlen(plainText), key, iv, &encrypted_text);

        if (encrypted_len > 0) {
            printf("Test Case %d:\n", i + 1);
            printf("  Plaintext:  %s\n", plainText);
            print_hex("  Encrypted (Hex): ", encrypted_text, encrypted_len);
            printf("\n");
            free(encrypted_text); // Free the memory allocated by the encrypt function
        } else {
            fprintf(stderr, "Encryption failed for test case %d.\n", i + 1);
        }
    }

    return 0;
}