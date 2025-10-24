// Note: This code requires the OpenSSL library.
// You need to link against it, e.g., gcc your_file.c -o your_app -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// 1. Create a static initialization vector (IV).
const unsigned char STATIC_IV[] = "abcdefghijklmnop";
// A secret key is also required for AES. Using a 128-bit key (16 bytes).
const unsigned char SECRET_KEY[] = "0123456789abcdef";

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
    abort();
}

/**
 * Encrypts plaintext. The caller must free the memory allocated for ciphertext.
 * @return The length of the ciphertext, or -1 on error.
 */
int encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key,
            const unsigned char *iv, unsigned char **ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    
    // The output buffer must be large enough for the plaintext plus one block for padding
    *ciphertext = (unsigned char *)malloc(plaintext_len + EVP_MAX_BLOCK_LENGTH);
    if (!*ciphertext) {
        fprintf(stderr, "Malloc failed.\n");
        return -1;
    }

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
    }

    // 2. Initialize a cipher using AES in CBC mode with the static IV.
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    
    // 3. Pad the input data to match the block size required by the cipher.
    // OpenSSL's EVP functions handle padding by default.
    // 4. Encrypt the padded data using the cipher.
    if(1 != EVP_EncryptUpdate(ctx, *ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, *ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    
    // 5. Return the encrypted data (via parameter) and its length.
    return ciphertext_len;
}

// Helper to print bytes as a hex string
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

void run_tests() {
    const char* test_cases[] = {
        "",
        "Hello",
        "16-byte message!", // Exactly 16 bytes, will add a full padding block
        "This is a longer test message for AES encryption.",
        "!@#$%^&*()_+"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("C AES/CBC Encryption Test Cases:\n");
    for (int i = 0; i < num_test_cases; ++i) {
        unsigned char* encrypted_data = NULL;
        int encrypted_len = encrypt(
            (const unsigned char*)test_cases[i],
            strlen(test_cases[i]),
            SECRET_KEY,
            STATIC_IV,
            &encrypted_data
        );

        if (encrypted_len >= 0) {
            printf("Test Case %d:\n", i + 1);
            printf("  Plaintext:  \"%s\"\n", test_cases[i]);
            print_hex("  Encrypted (Hex):    ", encrypted_data, encrypted_len);
            printf("\n");
            free(encrypted_data); // Free the allocated memory
        } else {
            fprintf(stderr, "Encryption failed for test case %d.\n", i + 1);
        }
    }
}

int main() {
    // In C, we don't have classes like Task72, so we run the tests directly.
    run_tests();
    return 0;
}