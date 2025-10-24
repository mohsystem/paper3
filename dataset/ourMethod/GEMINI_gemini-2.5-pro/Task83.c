#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Constants based on security rules
#define SALT_LENGTH_BYTES 16
#define IV_LENGTH_BYTES 12
#define TAG_LENGTH_BYTES 16
#define PBKDF2_ITERATIONS 210000
#define AES_KEY_BITS 256
#define AES_KEY_BYTES (AES_KEY_BITS / 8)

void handle_openssl_error() {
    fprintf(stderr, "OpenSSL error:\n");
    ERR_print_errors_fp(stderr);
}

// Helper to Base64 encode for printing. Caller must free the returned string.
char* to_base64(const unsigned char* data, size_t length) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    if (!bio || !b64) {
        if (bio) BIO_free(bio);
        if (b64) BIO_free(b64);
        handle_openssl_error();
        return NULL;
    }
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    BIO_write(bio, data, length);
    BIO_flush(bio);
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    char* result = (char*)malloc(buffer_ptr->length + 1);
    if (result) {
        memcpy(result, buffer_ptr->data, buffer_ptr->length);
        result[buffer_ptr->length] = '\0';
    }
    BIO_free_all(bio);
    return result;
}

/**
 * Encrypts plaintext using AES-256-GCM. The security rules mandate AES-GCM.
 * Returns a dynamically allocated buffer with the encrypted data.
 * The caller is responsible for freeing this buffer.
 */
unsigned char* encrypt_data(const unsigned char* plaintext, int plaintext_len, const char* password, int* out_len) {
    int success = 0;
    unsigned char salt[SALT_LENGTH_BYTES];
    unsigned char iv[IV_LENGTH_BYTES];
    unsigned char key[AES_KEY_BYTES];
    unsigned char tag[TAG_LENGTH_BYTES];
    
    unsigned char* ciphertext = NULL;
    unsigned char* result = NULL;
    EVP_CIPHER_CTX *ctx = NULL;

    if (RAND_bytes(salt, sizeof(salt)) != 1) { handle_openssl_error(); goto cleanup; }
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) { handle_openssl_error(); goto cleanup; }
    if (RAND_bytes(iv, sizeof(iv)) != 1) { handle_openssl_error(); goto cleanup; }

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { handle_openssl_error(); goto cleanup; }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) { handle_openssl_error(); goto cleanup; }
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL) != 1) { handle_openssl_error(); goto cleanup; }
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) { handle_openssl_error(); goto cleanup; }
    
    ciphertext = malloc(plaintext_len);
    if (!ciphertext) { perror("malloc for ciphertext"); goto cleanup; }
    
    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) { handle_openssl_error(); goto cleanup; }
    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) { handle_openssl_error(); goto cleanup; }
    ciphertext_len += len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1) { handle_openssl_error(); goto cleanup; }
    
    *out_len = sizeof(salt) + sizeof(iv) + ciphertext_len + sizeof(tag);
    result = malloc(*out_len);
    if (!result) { perror("malloc for result"); goto cleanup; }

    memcpy(result, salt, sizeof(salt));
    memcpy(result + sizeof(salt), iv, sizeof(iv));
    memcpy(result + sizeof(salt) + sizeof(iv), ciphertext, ciphertext_len);
    memcpy(result + sizeof(salt) + sizeof(iv) + ciphertext_len, tag, sizeof(tag));
    
    success = 1;

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (ciphertext) free(ciphertext);
    OPENSSL_cleanse(key, sizeof(key));

    if (!success) {
        free(result);
        result = NULL;
        *out_len = 0;
    }
    
    return result;
}

void run_test_case(int case_num, const char* original_key, const char* password) {
    printf("\n--- Test Case %d ---\n", case_num);
    printf("Original data: %s\n", original_key);

    int encrypted_len = 0;
    unsigned char* encrypted_data = encrypt_data((const unsigned char*)original_key, strlen(original_key), password, &encrypted_len);

    if (encrypted_data && encrypted_len > 0) {
        char* b64_str = to_base64(encrypted_data, encrypted_len);
        if (b64_str) {
            printf("Encrypted (Base64): %s\n", b64_str);
            free(b64_str);
        }
        printf("Encrypted data length: %d bytes\n", encrypted_len);
        free(encrypted_data);
    } else {
        fprintf(stderr, "Test Case %d failed.\n", case_num);
    }
}

int main() {
    // NOTE: Hardcoding secrets is insecure and violates security rules.
    // This is for demonstration purposes only. In a real application,
    // use a secure mechanism like environment variables or a secrets vault.
    const char* password = "very-strong-password-123";
    const char* test_keys_to_encrypt[] = {
        "0123456789abcdef0123456789abcdef",
        "short key",
        "a-longer-key-with-some-symbols!@#$",
        "", // Empty string
        "another key for testing purposes 12345"
    };
    int num_tests = sizeof(test_keys_to_encrypt) / sizeof(test_keys_to_encrypt[0]);

    printf("Running 5 test cases for AES-GCM encryption...\n");

    for (int i = 0; i < num_tests; ++i) {
        run_test_case(i + 1, test_keys_to_encrypt[i], password);
    }
    
    return 0;
}