#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Note: To compile this code, you need to link against OpenSSL libraries.
// Example: gcc your_file.c -o your_program -lssl -lcrypto

// Helper function to print a byte array as a hex string
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * @brief Encrypts data using AES-256-GCM.
 * 
 * @param plain_text The plaintext to encrypt.
 * @param plain_text_len The length of the plaintext.
 * @param key The 32-byte (256-bit) secret key.
 * @param encrypted_message A pointer that will be set to the newly allocated encrypted message ([IV][Ciphertext][Tag]). The caller must free this memory.
 * @param encrypted_message_len A pointer to an integer that will hold the length of the encrypted message.
 * @return 0 on success, -1 on failure.
 */
int encrypt(const unsigned char* plain_text, int plain_text_len, const unsigned char* key, 
            unsigned char** encrypted_message, int* encrypted_message_len) {
    
    const int IV_LENGTH = 12;
    const int TAG_LENGTH = 16;
    int ret = -1; // Default to error

    // Allocate buffer for ciphertext. In GCM, ciphertext length is same as plaintext length.
    unsigned char* ciphertext = (unsigned char*)malloc(plain_text_len);
    if (!ciphertext) return -1;

    unsigned char iv[IV_LENGTH];
    unsigned char tag[TAG_LENGTH];
    
    EVP_CIPHER_CTX *ctx = NULL;
    int len;
    int ciphertext_len;

    // 1. Generate random IV
    if (1 != RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "RAND_bytes failed.\n");
        goto cleanup;
    }

    // 2. Create and initialize the cipher context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        fprintf(stderr, "EVP_CIPHER_CTX_new failed.\n");
        goto cleanup;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv)) {
        fprintf(stderr, "EVP_EncryptInit_ex failed for key and IV.\n");
        goto cleanup;
    }

    // 3. Encrypt the message
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plain_text, plain_text_len)) {
        fprintf(stderr, "EVP_EncryptUpdate failed.\n");
        goto cleanup;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        fprintf(stderr, "EVP_EncryptFinal_ex failed.\n");
        goto cleanup;
    }
    ciphertext_len += len;

    // Get the authentication tag
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag)) {
        fprintf(stderr, "EVP_CTRL_GCM_GET_TAG failed.\n");
        goto cleanup;
    }

    // Combine IV, ciphertext, and tag into a single buffer
    *encrypted_message_len = IV_LENGTH + ciphertext_len + TAG_LENGTH;
    *encrypted_message = (unsigned char*)malloc(*encrypted_message_len);
    if (!*encrypted_message) {
        fprintf(stderr, "Malloc for encrypted_message failed.\n");
        goto cleanup;
    }

    memcpy(*encrypted_message, iv, IV_LENGTH);
    memcpy(*encrypted_message + IV_LENGTH, ciphertext, ciphertext_len);
    memcpy(*encrypted_message + IV_LENGTH + ciphertext_len, tag, TAG_LENGTH);

    ret = 0; // Success

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (ciphertext) free(ciphertext);
    return ret;
}

int main() {
    // In a real application, this key should be managed securely.
    unsigned char key[32]; // 256-bit key
    if (!RAND_bytes(key, sizeof(key))) {
        fprintf(stderr, "Failed to generate random key.\n");
        return 1;
    }
    print_hex("Using Secret Key (Hex)", key, sizeof(key));
    printf("---\n");

    const char* test_cases[] = {
        "This is a secret message.",
        "Hello, World!",
        "Cryptography is fascinating.",
        "AES-GCM provides authenticated encryption.",
        "12345!@#$%^"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        printf("Test Case %d\n", i + 1);
        printf("Original:  %s\n", test_cases[i]);

        unsigned char* encrypted_message = NULL;
        int encrypted_len = 0;
        
        if (encrypt((unsigned char*)test_cases[i], strlen(test_cases[i]), key, 
                    &encrypted_message, &encrypted_len) == 0) {
            print_hex("Encrypted (Hex)", encrypted_message, encrypted_len);
            free(encrypted_message); // Free the memory allocated by the encrypt function
        } else {
            fprintf(stderr, "Encryption failed.\n");
        }
        printf("\n");
    }

    return 0;
}