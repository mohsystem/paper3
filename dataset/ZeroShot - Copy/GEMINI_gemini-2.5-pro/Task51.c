#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Note: This code requires linking with OpenSSL libraries.
// Example compilation command: gcc your_file.c -o your_program -lssl -lcrypto

// AES-GCM parameters
#define AES_KEY_SIZE 32   // 256 bits
#define GCM_IV_LENGTH 12  // 96 bits
#define GCM_TAG_LENGTH 16 // 128 bits

/**
 * Base64 encodes a byte array.
 * The caller is responsible for freeing the returned string using free().
 */
char* base64_encode(const unsigned char* input, int length) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);

    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    char* encoded_data = (char*)malloc(buffer_ptr->length + 1);
    if (encoded_data) {
        memcpy(encoded_data, buffer_ptr->data, buffer_ptr->length);
        encoded_data[buffer_ptr->length] = '\0';
    }

    BIO_free_all(bio);
    return encoded_data;
}

/**
 * Encrypts a plaintext string using AES-256-GCM.
 * The output is a Base64 encoded string containing the IV, ciphertext, and tag.
 *
 * @param plaintext The null-terminated string to encrypt.
 * @param key The 32-byte secret key.
 * @return A Base64 encoded string representing the encrypted data.
 *         The caller is responsible for freeing this string using free().
 *         Returns NULL on failure.
 */
char* encrypt_data(const char* plaintext, const unsigned char* key) {
    int plaintext_len = strlen(plaintext);
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char* ciphertext = NULL;
    unsigned char* cipher_message = NULL;
    char* base64_encoded = NULL;
    int len, ciphertext_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto cleanup;

    unsigned char iv[GCM_IV_LENGTH];
    if (1 != RAND_bytes(iv, sizeof(iv))) goto cleanup;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv)) goto cleanup;

    ciphertext = (unsigned char*)malloc(plaintext_len);
    if (!ciphertext) goto cleanup;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)plaintext, plaintext_len)) goto cleanup;
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) goto cleanup;
    ciphertext_len += len;
    
    unsigned char tag[GCM_TAG_LENGTH];
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) goto cleanup;

    int cipher_message_len = GCM_IV_LENGTH + ciphertext_len + GCM_TAG_LENGTH;
    cipher_message = (unsigned char*)malloc(cipher_message_len);
    if (!cipher_message) goto cleanup;

    memcpy(cipher_message, iv, GCM_IV_LENGTH);
    memcpy(cipher_message + GCM_IV_LENGTH, ciphertext, ciphertext_len);
    memcpy(cipher_message + GCM_IV_LENGTH + ciphertext_len, tag, GCM_TAG_LENGTH);

    base64_encoded = base64_encode(cipher_message, cipher_message_len);

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (ciphertext) free(ciphertext);
    if (cipher_message) free(cipher_message);
    if (!base64_encoded) {
      fprintf(stderr, "Error: Encryption failed.\n");
    }
    return base64_encoded;
}

int main() {
    // IMPORTANT: In a real-world application, the key must be securely
    // managed and not hardcoded or generated on the fly like this.
    // This is for demonstration purposes only.
    unsigned char key[AES_KEY_SIZE];
    if (1 != RAND_bytes(key, sizeof(key))) {
        fprintf(stderr, "Failed to generate key.\n");
        return 1;
    }
    
    char* key_b64 = base64_encode(key, sizeof(key));
    if(key_b64) {
        printf("Using AES Key (Base64): %s\n", key_b64);
        free(key_b64);
    }

    const char* testCases[] = {
        "This is a secret message.",
        "1234567890",
        "!@#$%^&*()_+",
        "A very long message to test the encryption with a larger payload to see how it performs and handles bigger data chunks.",
        "" // Empty string
    };
    int num_test_cases = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_test_cases; i++) {
        const char* plaintext = testCases[i];
        printf("\nTest Case %d\n", i + 1);
        printf("Plaintext: %s\n", plaintext);
        
        char* encryptedText = encrypt_data(plaintext, key);
        if (encryptedText) {
            printf("Encrypted (Base64): %s\n", encryptedText);
            free(encryptedText);
        }
    }

    return 0;
}