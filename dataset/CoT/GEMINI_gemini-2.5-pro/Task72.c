#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// IMPORTANT: Using a static, predictable IV is a major security vulnerability
// in modes like CBC. The IV should be random and unique for each encryption
// operation with the same key. This code is for demonstration purposes only,
// following the prompt's specific requirements.
static const unsigned char STATIC_IV[] = "1234567890123456";

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
}

/**
 * Encrypts data using AES in CBC mode with a static IV.
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the plaintext data.
 * @param key The secret key for encryption (must be 16 bytes for AES-128).
 * @param ciphertext Buffer where the encrypted data will be written. Must be large enough.
 * @return The length of the encrypted data, or -1 on failure.
 */
int encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, 
            unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
        return -1;
    }

    // Initialize the cipher using AES in CBC mode with the static IV.
    // OpenSSL's EVP_* encrypt functions handle PKCS#7 padding by default.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Encrypt the padded data.
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize the encryption (handles final block and padding).
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// Helper to print byte array as a hex string
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    // To compile: gcc your_file_name.c -o your_program_name -lssl -lcrypto
    
    // IMPORTANT: Hardcoding keys is a security risk. In a real application,
    // use a secure key management system.
    // 16 bytes for AES-128
    const unsigned char key[] = "thisisakeyjklmno";

    const char* test_cases[] = {
        "This is test case 1.",
        "Another test case with different data.",
        "Short",
        "A much longer string to test padding and multiple blocks of encryption.",
        "Test case 5 is the final one!"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("Encrypting using AES/CBC with a STATIC IV (INSECURE PRACTICE)\n");
    printf("==========================================================\n");

    for (int i = 0; i < num_test_cases; ++i) {
        const char* plain_text = test_cases[i];
        int plain_text_len = strlen(plain_text);

        // Allocate buffer for ciphertext. It must be large enough for the
        // plaintext plus one block for padding (EVP_MAX_BLOCK_LENGTH).
        unsigned char* ciphertext = malloc(plain_text_len + EVP_MAX_BLOCK_LENGTH);
        if (!ciphertext) {
            fprintf(stderr, "Failed to allocate memory for ciphertext.\n");
            return 1;
        }

        int ciphertext_len = encrypt((const unsigned char*)plain_text, plain_text_len, key, ciphertext);

        printf("Test Case %d:\n", i + 1);
        printf("  Plaintext:  %s\n", plain_text);

        if (ciphertext_len >= 0) {
            print_hex("  Encrypted (Hex): ", ciphertext, ciphertext_len);
        } else {
            printf("  Encryption failed.\n");
        }
        printf("\n");
        
        free(ciphertext);
    }

    return 0;
}