#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/kdf.h>

// Constants based on security requirements
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32 // 256 bits for AES-256
#define PBKDF2_ITERATIONS 210000

/**
 * Encrypts plaintext using AES-256-GCM.
 * The returned buffer is dynamically allocated and must be freed by the caller.
 * The output format is [salt(16)][iv(12)][ciphertext][tag(16)].
 *
 * @param plaintext The data to encrypt.
 * @param plaintext_len The length of the plaintext.
 * @param password The password to derive the key from.
 * @param encrypted_data_len A pointer to an int to store the length of the encrypted data.
 * @return A pointer to the encrypted data on success, NULL on failure.
 */
unsigned char* encrypt_string(const unsigned char* plaintext, int plaintext_len,
                             const char* password, int* encrypted_data_len) {
    if (plaintext == NULL || password == NULL || encrypted_data_len == NULL) {
        return NULL;
    }

    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char tag[TAG_SIZE];
    unsigned char key[KEY_SIZE];

    EVP_CIPHER_CTX* ctx = NULL;
    unsigned char* ciphertext = NULL;
    unsigned char* result = NULL;
    int len = 0;
    int ciphertext_len = 0;
    int success = 0;

    // 1. Generate a random salt
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate salt.\n");
        goto cleanup;
    }

    // 2. Derive encryption key from password and salt using PBKDF2-HMAC-SHA256
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt),
                          PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) {
        fprintf(stderr, "Failed to derive key.\n");
        goto cleanup;
    }

    // 3. Generate a random IV
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        fprintf(stderr, "Failed to generate IV.\n");
        goto cleanup;
    }

    // 4. Create the cipher context
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context.\n");
        goto cleanup;
    }

    // 5. Initialize AES-256-GCM encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        fprintf(stderr, "Failed to initialize encryption.\n");
        goto cleanup;
    }

    // 6. Provide the plaintext to be encrypted
    // For GCM, the ciphertext length is equal to the plaintext length.
    ciphertext = malloc(plaintext_len > 0 ? plaintext_len : 1);
    if (!ciphertext) {
        fprintf(stderr, "Failed to allocate memory for ciphertext.\n");
        goto cleanup;
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        fprintf(stderr, "Failed to encrypt plaintext.\n");
        goto cleanup;
    }
    ciphertext_len = len;

    // 7. Finalize the encryption.
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Failed to finalize encryption.\n");
        goto cleanup;
    }
    ciphertext_len += len;

    // 8. Get the GCM authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1) {
        fprintf(stderr, "Failed to get GCM tag.\n");
        goto cleanup;
    }

    // 9. Combine salt, IV, ciphertext, and tag into a single buffer
    *encrypted_data_len = sizeof(salt) + sizeof(iv) + ciphertext_len + sizeof(tag);
    result = malloc(*encrypted_data_len);
    if (!result) {
        fprintf(stderr, "Failed to allocate memory for result.\n");
        goto cleanup;
    }

    memcpy(result, salt, sizeof(salt));
    memcpy(result + sizeof(salt), iv, sizeof(iv));
    memcpy(result + sizeof(salt) + sizeof(iv), ciphertext, ciphertext_len);
    memcpy(result + sizeof(salt) + sizeof(iv) + ciphertext_len, tag, sizeof(tag));

    success = 1; // Mark as successful

cleanup:
    // Securely clear the key from memory
    OPENSSL_cleanse(key, sizeof(key));

    if (ctx) {
        EVP_CIPHER_CTX_free(ctx);
    }
    if (ciphertext) {
        free(ciphertext);
    }

    if (!success) { // If there was an error
        if (result) {
            free(result);
        }
        return NULL;
    }

    return result;
}

void print_hex(const char* label, const unsigned char* data, int len) {
    if (data == NULL || len <= 0) {
        printf("%s: (null or empty)\n", label);
        return;
    }
    printf("%s: ", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}


void run_test_case(int case_num, const char* plaintext, const char* password) {
    printf("--- Test Case %d ---\n", case_num);
    if (strlen(plaintext) == 0) {
         printf("Plaintext: (empty)\n");
    } else if (strlen(plaintext) > 100) {
        printf("Plaintext: (long string, length %zu)\n", strlen(plaintext));
    } else {
         printf("Plaintext: %s\n", plaintext);
    }


    int encrypted_len = 0;
    unsigned char* encrypted_data = encrypt_string(
        (const unsigned char*)plaintext, strlen(plaintext), password, &encrypted_len);

    if (encrypted_data) {
        print_hex("Encrypted", encrypted_data, encrypted_len);
        free(encrypted_data);
    } else {
        printf("Encryption failed.\n");
        // Print OpenSSL error stack if available
        ERR_print_errors_fp(stderr);
    }
    printf("\n");
}


int main() {
    run_test_case(1, "This is a secret message.", "strong_password123");
    run_test_case(2, "Another secret.", "another_Pa$$w0rd!");
    run_test_case(3, "", "passwordForEmpty");

    char long_plaintext[1001];
    memset(long_plaintext, 'A', 1000);
    long_plaintext[1000] = '\0';
    run_test_case(4, long_plaintext, "long_and_secure_password_for_a_long_message");
    
    run_test_case(5, "UTF-8 test: 😁 $¢€", "special-chars-@#$%-password");

    return 0;
}