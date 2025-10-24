/*
 * To compile and run this code, you need to have OpenSSL installed.
 *
 * For Debian/Ubuntu: sudo apt-get install libssl-dev
 * For RHEL/CentOS: sudo yum install openssl-devel
 * For macOS (with Homebrew): brew install openssl
 *
 * Compile command:
 * gcc Task79.c -o Task79 -lssl -lcrypto
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants
#define AES_KEY_SIZE 256
#define AES_KEY_BYTES (AES_KEY_SIZE / 8)
#define GCM_IV_LENGTH 12
#define GCM_TAG_LENGTH 16

void handle_openssl_errors(void) {
    // In a real application, you would log these errors properly.
    fprintf(stderr, "An OpenSSL error occurred.\n");
    // ERR_print_errors_fp(stderr); // You can uncomment this for more detailed errors
}

// Simple Base64 encode function using OpenSSL
char* base64_encode(const unsigned char* input, int length) {
    const int pl = 4 * ((length + 2) / 3);
    char* output = malloc(pl + 1);
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory for base64 encoding.\n");
        return NULL;
    }
    const int ol = EVP_EncodeBlock((unsigned char*)output, input, length);
    if (pl != ol) {
        fprintf(stderr, "Base64 encoding length mismatch.\n");
        free(output);
        return NULL;
    }
    output[pl] = '\0';
    return output;
}

/**
 * Encrypts a message using AES-256-GCM.
 *
 * @param message The plaintext message to encrypt.
 * @param message_len The length of the plaintext message.
 * @param key The 32-byte secret key.
 * @param encrypted_message A pointer to a buffer that will be allocated and filled with the encrypted data [IV][Ciphertext][Tag].
 * @return The length of the encrypted message, or -1 on failure.
 */
int encrypt(const unsigned char* message, int message_len, const unsigned char* key, unsigned char** encrypted_message) {
    EVP_CIPHER_CTX* ctx = NULL;
    int len = 0;
    int ciphertext_len = 0;

    // 1. Create a Cipher for encryption
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
        return -1;
    }

    // 2. Generate a secure random IV
    unsigned char iv[GCM_IV_LENGTH];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // 3. Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_LENGTH, NULL) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // 4. Allocate memory and encrypt the message
    unsigned char* ciphertext = malloc(message_len);
    if (!ciphertext) {
        fprintf(stderr, "Failed to allocate memory for ciphertext.\n");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, message, message_len) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    // Get authentication tag
    unsigned char tag[GCM_TAG_LENGTH];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    EVP_CIPHER_CTX_free(ctx);

    // 5. Concatenate IV, ciphertext, and tag
    int encrypted_len = GCM_IV_LENGTH + ciphertext_len + GCM_TAG_LENGTH;
    *encrypted_message = malloc(encrypted_len);
    if (!*encrypted_message) {
        fprintf(stderr, "Failed to allocate memory for encrypted message.\n");
        free(ciphertext);
        return -1;
    }
    memcpy(*encrypted_message, iv, GCM_IV_LENGTH);
    memcpy(*encrypted_message + GCM_IV_LENGTH, ciphertext, ciphertext_len);
    memcpy(*encrypted_message + GCM_IV_LENGTH + ciphertext_len, tag, GCM_TAG_LENGTH);

    free(ciphertext);
    return encrypted_len;
}

int main() {
    // Generate a new AES secret key
    unsigned char secret_key[AES_KEY_BYTES];
    if (!RAND_bytes(secret_key, sizeof(secret_key))) {
        fprintf(stderr, "Failed to generate secret key.\n");
        return 1;
    }

    char* key_b64 = base64_encode(secret_key, AES_KEY_BYTES);
    printf("Generated AES-256 Key (Base64): %s\n", key_b64);
    free(key_b64);
    printf("--- Running 5 Test Cases ---\n");

    const char* test_messages[] = {
        "This is the first secret message.",
        "Security is important for all applications.",
        "AES-GCM provides authenticated encryption.",
        "1234567890!@#$%^&*()_+",
        "A much longer message to test the encryption process to ensure it works correctly with varying lengths of input data. The quick brown fox jumps over the lazy dog."
    };
    int num_tests = sizeof(test_messages) / sizeof(test_messages[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* message = test_messages[i];
        unsigned char* encrypted_data = NULL;
        int encrypted_len = -1;

        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Original Message: %s\n", message);
        
        encrypted_len = encrypt((unsigned char*)message, strlen(message), secret_key, &encrypted_data);

        if (encrypted_len > 0 && encrypted_data != NULL) {
            char* encrypted_b64 = base64_encode(encrypted_data, encrypted_len);
            if (encrypted_b64) {
                printf("Encrypted (Base64): %s\n", encrypted_b64);
                free(encrypted_b64);
            }
            free(encrypted_data);
        } else {
            printf("Encryption failed for test case %d.\n", i + 1);
        }
    }

    return 0;
}