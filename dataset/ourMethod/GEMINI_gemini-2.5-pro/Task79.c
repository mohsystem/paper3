#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Constants
#define SALT_SIZE 16
#define IV_SIZE 12
#define KEY_SIZE 32 // 256 bits
#define TAG_SIZE 16 // 128 bits
#define PBKDF2_ITERATIONS 210000

// Helper to print binary data as hex
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Encrypts a plaintext message using a password.
 *
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the plaintext data.
 * @param password The password to derive the key from.
 * @param encrypted_data_out Pointer to be populated with the allocated encrypted data.
 *                           The caller is responsible for freeing this memory.
 * @return The length of the encrypted data on success, or -1 on failure.
 */
int encrypt_message(const unsigned char* plaintext, int plaintext_len, const char* password, unsigned char** encrypted_data_out) {
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE];
    unsigned char tag[TAG_SIZE];
    
    unsigned char* ciphertext = NULL;
    unsigned char* result = NULL;
    int len;
    int ciphertext_len = 0;
    int result_len = -1;

    EVP_CIPHER_CTX *ctx = NULL;

    // 1. Generate salt and IV
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate salt.\n");
        goto cleanup;
    }
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        fprintf(stderr, "Failed to generate IV.\n");
        goto cleanup;
    }

    // 2. Derive key
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) {
        fprintf(stderr, "Failed to derive key.\n");
        goto cleanup;
    }

    // 3. Encrypt
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        fprintf(stderr, "Failed to create cipher context.\n");
        goto cleanup;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        fprintf(stderr, "Failed to initialize cipher.\n");
        goto cleanup;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL) != 1) {
        fprintf(stderr, "Failed to set IV length.\n");
        goto cleanup;
    }

    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        fprintf(stderr, "Failed to set key and IV.\n");
        goto cleanup;
    }
    
    ciphertext = malloc(plaintext_len);
    if (!ciphertext) {
        fprintf(stderr, "Failed to allocate memory for ciphertext.\n");
        goto cleanup;
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        fprintf(stderr, "Encryption update failed.\n");
        goto cleanup;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Encryption finalization failed.\n");
        goto cleanup;
    }
    ciphertext_len += len;
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1) {
        fprintf(stderr, "Failed to get GCM tag.\n");
        goto cleanup;
    }

    // 4. Concatenate: salt || iv || ciphertext || tag
    result_len = sizeof(salt) + sizeof(iv) + ciphertext_len + sizeof(tag);
    result = malloc(result_len);
    if (!result) {
        fprintf(stderr, "Failed to allocate memory for result.\n");
        result_len = -1;
        goto cleanup;
    }
    
    memcpy(result, salt, sizeof(salt));
    memcpy(result + sizeof(salt), iv, sizeof(iv));
    memcpy(result + sizeof(salt) + sizeof(iv), ciphertext, ciphertext_len);
    memcpy(result + sizeof(salt) + sizeof(iv) + ciphertext_len, tag, sizeof(tag));
    
    *encrypted_data_out = result;

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (ciphertext) free(ciphertext);
    // Securely clear key
    OPENSSL_cleanse(key, sizeof(key));
    if (result_len == -1 && result) {
        free(result);
        *encrypted_data_out = NULL;
    }

    return result_len;
}

int main() {
    const char* password = "aVeryStrongPassword!123";
    
    const char* test_cases[] = {
        "Hello, World!",
        "This is a secret message.",
        "", // Empty string
        "Short",
        "A much longer message to test the encryption process with more blocks of data to see how it handles it."
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        const char* plaintext_str = test_cases[i];
        printf("Plaintext: %s\n", plaintext_str);

        unsigned char* encrypted_data = NULL;
        int encrypted_len = encrypt_message((const unsigned char*)plaintext_str, strlen(plaintext_str), password, &encrypted_data);

        if (encrypted_len > 0 && encrypted_data) {
            print_hex("Encrypted (Hex): ", encrypted_data, encrypted_len);
            free(encrypted_data);
        } else {
            fprintf(stderr, "Encryption failed.\n");
            ERR_print_errors_fp(stderr);
        }
        printf("\n");
    }

    return 0;
}