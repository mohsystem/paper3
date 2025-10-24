#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// To compile: gcc your_file.c -o your_app -lssl -lcrypto
// Make sure you have OpenSSL development libraries installed (e.g., libssl-dev on Debian/Ubuntu).

// Static Initialization Vector (IV). For AES, this is 16 bytes (128 bits).
// In a real-world application, IV should be random and unique for each encryption.
const unsigned char STATIC_IV[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
}

/**
 * Encrypts data using AES-128-CBC with a static IV.
 * The caller is responsible for freeing the returned ciphertext buffer.
 * PKCS padding is enabled by default in EVP_EncryptInit_ex.
 *
 * @param key The 16-byte (128-bit) encryption key.
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the plaintext data.
 * @param ciphertext_len A pointer to an integer where the length of the ciphertext will be stored.
 * @return A dynamically allocated buffer containing the encrypted data, or NULL on failure.
 */
unsigned char* encrypt(const unsigned char* key, const unsigned char* plaintext, int plaintext_len, int* ciphertext_len) {
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char* ciphertext = NULL;
    int len = 0;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
        return NULL;
    }

    // Initialize the encryption operation.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    // Allocate buffer for ciphertext. It can be up to one block larger than plaintext.
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    ciphertext = (unsigned char*)malloc(plaintext_len + block_size);
    if (!ciphertext) {
        perror("malloc failed");
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    // Provide the message to be encrypted, and obtain the encrypted output.
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    *ciphertext_len = len;

    // Finalize the encryption. This handles padding.
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    *ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    // A 16-byte (128-bit) key for demonstration.
    // In a real application, use a secure key management system.
    const unsigned char key[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };

    const char* test_cases[] = {
        "Hello, World!",
        "16-byte message!", // Exactly one block
        "This is a longer test message for AES CBC encryption.",
        "", // Empty string
        "123!@#$%^&*()"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("C AES/CBC Encryption with Static IV\n");
    printf("-----------------------------------\n");

    for (int i = 0; i < num_test_cases; ++i) {
        printf("Test Case #%d\n", i + 1);
        printf("Plaintext: %s\n", test_cases[i]);
        
        int ciphertext_len;
        unsigned char* encrypted_data = encrypt(
            key, 
            (const unsigned char*)test_cases[i], 
            strlen(test_cases[i]), 
            &ciphertext_len
        );

        if (encrypted_data) {
            print_hex("Encrypted (Hex): ", encrypted_data, ciphertext_len);
            free(encrypted_data); // Free the allocated memory
        } else {
            printf("Encryption failed.\n");
        }
        printf("\n");
    }

    return 0;
}