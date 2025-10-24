
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#define KEY_LENGTH 32
#define ITERATION_COUNT 65536
#define SALT_LENGTH 16
#define IV_LENGTH 16

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    char* result = (char*)malloc(buffer_ptr->length + 1);
    memcpy(result, buffer_ptr->data, buffer_ptr->length);
    result[buffer_ptr->length] = '\\0';

    BIO_free_all(bio);
    return result;
}

char* encrypt_message(const char* message, const char* secret_key) {
    unsigned char salt[SALT_LENGTH];
    unsigned char key[KEY_LENGTH];
    unsigned char iv[IV_LENGTH];
    
    // Generate random salt
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate salt\\n");
        return NULL;
    }

    // Derive key from password using PBKDF2
    if (PKCS5_PBKDF2_HMAC(secret_key, strlen(secret_key),
                          salt, SALT_LENGTH,
                          ITERATION_COUNT,
                          EVP_sha256(),
                          KEY_LENGTH, key) != 1) {
        fprintf(stderr, "Key derivation failed\\n");
        return NULL;
    }

    // Generate random IV
    if (RAND_bytes(iv, IV_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate IV\\n");
        return NULL;
    }

    // Create cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context\\n");
        return NULL;
    }

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "Encryption initialization failed\\n");
        return NULL;
    }

    // Encrypt message
    int message_len = strlen(message);
    unsigned char* ciphertext = (unsigned char*)malloc(message_len + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message, message_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        fprintf(stderr, "Encryption update failed\\n");
        return NULL;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        fprintf(stderr, "Encryption finalization failed\\n");
        return NULL;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Combine salt + IV + encrypted data
    size_t combined_len = SALT_LENGTH + IV_LENGTH + ciphertext_len;
    unsigned char* combined = (unsigned char*)malloc(combined_len);
    memcpy(combined, salt, SALT_LENGTH);
    memcpy(combined + SALT_LENGTH, iv, IV_LENGTH);
    memcpy(combined + SALT_LENGTH + IV_LENGTH, ciphertext, ciphertext_len);

    // Encode to Base64
    char* result = base64_encode(combined, combined_len);

    free(ciphertext);
    free(combined);

    return result;
}

int main() {
    const char* messages[] = {
        "Hello, World!",
        "Secure message 123",
        "This is a confidential document",
        "Test@#$%^&*()",
        "Another secret message"
    };
    
    const char* keys[] = {
        "MySecretKey123",
        "P@ssw0rd!Strong",
        "EncryptionKey456",
        "SecurePassword789",
        "TopSecret2024"
    };

    printf("=== Encryption Test Cases ===\\n\\n");
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        printf("Original Message: %s\\n", messages[i]);
        printf("Secret Key: %s\\n", keys[i]);
        
        char* encrypted = encrypt_message(messages[i], keys[i]);
        if (encrypted) {
            printf("Encrypted Message: %s\\n", encrypted);
            printf("Encrypted Length: %zu characters\\n", strlen(encrypted));
            free(encrypted);
        } else {
            printf("Encryption failed\\n");
        }
        printf("\\n");
    }

    return 0;
}
