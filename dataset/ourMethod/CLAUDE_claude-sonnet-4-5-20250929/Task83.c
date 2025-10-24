
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>

/* Security constants */
#define SALT_LENGTH 16
#define IV_LENGTH 16
#define HMAC_LENGTH 32
#define PBKDF2_ITERATIONS 210000
#define AES_KEY_LENGTH 32
#define MAX_PLAINTEXT_SIZE (10 * 1024 * 1024)
#define MAGIC "ENC1"
#define VERSION 1

/* Structure to hold encrypted data */
typedef struct {
    unsigned char *data;
    size_t length;
} EncryptedData;

/**
 * Securely clears memory using OPENSSL_cleanse
 */
static void secure_clear(void *ptr, size_t len) {
    if (ptr != NULL && len > 0) {
        OPENSSL_cleanse(ptr, len);
    }
}

/**
 * Derives a 256-bit encryption key from a passphrase using PBKDF2-HMAC-SHA256.
 * Returns 1 on success, 0 on failure.
 */
static int derive_key(const char *passphrase, const unsigned char *salt,
                      size_t salt_len, unsigned char *key, size_t key_len) {
    /* Validate inputs - treat all inputs as untrusted */
    if (passphrase == NULL || strlen(passphrase) == 0) {
        fprintf(stderr, "Error: Passphrase must not be empty\\n");
        return 0;
    }
    if (salt == NULL || salt_len != SALT_LENGTH) {
        fprintf(stderr, "Error: Salt must be exactly 16 bytes\\n");
        return 0;
    }
    if (key == NULL || key_len != AES_KEY_LENGTH) {
        fprintf(stderr, "Error: Invalid key buffer\\n");
        return 0;
    }
    
    /* Use PBKDF2 with HMAC-SHA256 for secure key derivation */
    int result = PKCS5_PBKDF2_HMAC(
        passphrase,
        (int)strlen(passphrase),
        salt,
        (int)salt_len,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        (int)key_len,
        key
    );
    
    return result;
}

/**
 * Computes HMAC-SHA256 for Encrypt-then-MAC pattern.
 * Returns 1 on success, 0 on failure.
 */
static int compute_hmac(const unsigned char *key, size_t key_len,
                       const unsigned char *data, size_t data_len,
                       unsigned char *hmac_out, unsigned int *hmac_len) {
    unsigned char *result = HMAC(
        EVP_sha256(),
        key,
        (int)key_len,
        data,
        data_len,
        hmac_out,
        hmac_len
    );
    
    return (result != NULL && *hmac_len == HMAC_LENGTH) ? 1 : 0;
}

/**
 * Encrypts data using AES-256-CBC with HMAC-SHA256 (Encrypt-then-MAC).
 * Returns pointer to EncryptedData on success, NULL on failure.
 * Caller must free the returned structure and its data field.
 */
EncryptedData* encrypt_aes_cbc_with_hmac(const unsigned char *plaintext,
                                         size_t plaintext_len,
                                         const char *passphrase) {
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char salt[SALT_LENGTH] = {0};
    unsigned char hmac_salt[SALT_LENGTH] = {0};
    unsigned char iv[IV_LENGTH] = {0};
    unsigned char encryption_key[AES_KEY_LENGTH] = {0};
    unsigned char hmac_key[AES_KEY_LENGTH] = {0};
    unsigned char *ciphertext = NULL;
    unsigned char *result_data = NULL;
    EncryptedData *result = NULL;
    int len = 0;
    int ciphertext_len = 0;
    size_t result_len = 0;
    size_t offset = 0;
    unsigned char hmac_tag[HMAC_LENGTH] = {0};
    unsigned int hmac_len = 0;
    
    /* Input validation - treat all inputs as untrusted */
    if (plaintext == NULL || plaintext_len == 0) {
        fprintf(stderr, "Error: Plaintext must not be empty\\n");
        return NULL;
    }
    if (passphrase == NULL || strlen(passphrase) == 0) {
        fprintf(stderr, "Error: Passphrase must not be empty\\n");
        return NULL;
    }
    if (plaintext_len > MAX_PLAINTEXT_SIZE) {
        fprintf(stderr, "Error: Plaintext exceeds maximum size\\n");
        return NULL;
    }
    
    /* Generate cryptographically secure random salt and IV */
    if (RAND_bytes(salt, SALT_LENGTH) != 1 ||
        RAND_bytes(hmac_salt, SALT_LENGTH) != 1 ||
        RAND_bytes(iv, IV_LENGTH) != 1) {
        fprintf(stderr, "Error: Failed to generate random bytes\\n");
        goto cleanup;
    }
    
    /* Derive encryption key using secure KDF */
    if (!derive_key(passphrase, salt, SALT_LENGTH, encryption_key, AES_KEY_LENGTH)) {
        fprintf(stderr, "Error: Failed to derive encryption key\\n");
        goto cleanup;
    }
    
    /* Derive separate HMAC key for Encrypt-then-MAC */
    if (!derive_key(passphrase, hmac_salt, SALT_LENGTH, hmac_key, AES_KEY_LENGTH)) {
        fprintf(stderr, "Error: Failed to derive HMAC key\\n");
        goto cleanup;
    }
    
    /* Create and initialize encryption context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error: Failed to create cipher context\\n");
        goto cleanup;
    }
    
    /* Initialize AES-256-CBC encryption */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, encryption_key, iv) != 1) {
        fprintf(stderr, "Error: Failed to initialize encryption\\n");
        goto cleanup;
    }
    
    /* Allocate buffer for ciphertext (includes padding) */
    ciphertext = (unsigned char*)malloc(plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    if (ciphertext == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        goto cleanup;
    }
    
    /* Encrypt the plaintext */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)plaintext_len) != 1) {
        fprintf(stderr, "Error: Encryption failed\\n");
        goto cleanup;
    }
    ciphertext_len = len;
    
    /* Finalize encryption (handles padding) */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Error: Encryption finalization failed\\n");
        goto cleanup;
    }
    ciphertext_len += len;
    
    /* Calculate total result size */
    result_len = 4 + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + ciphertext_len + HMAC_LENGTH;
    
    /* Allocate result buffer */
    result_data = (unsigned char*)malloc(result_len);
    if (result_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        goto cleanup;
    }
    
    /* Build the output structure */
    memcpy(result_data + offset, MAGIC, 4);
    offset += 4;
    
    result_data[offset] = VERSION;
    offset += 1;
    
    memcpy(result_data + offset, salt, SALT_LENGTH);
    offset += SALT_LENGTH;
    
    memcpy(result_data + offset, hmac_salt, SALT_LENGTH);
    offset += SALT_LENGTH;
    
    memcpy(result_data + offset, iv, IV_LENGTH);
    offset += IV_LENGTH;
    
    memcpy(result_data + offset, ciphertext, ciphertext_len);
    offset += ciphertext_len;
    
    /* Compute HMAC over all data (Encrypt-then-MAC pattern) */
    if (!compute_hmac(hmac_key, AES_KEY_LENGTH, result_data, offset, hmac_tag, &hmac_len)) {
        fprintf(stderr, "Error: HMAC computation failed\\n");
        goto cleanup;
    }
    
    /* Add HMAC tag */
    memcpy(result_data + offset, hmac_tag, HMAC_LENGTH);
    
    /* Allocate and populate result structure */
    result = (EncryptedData*)malloc(sizeof(EncryptedData));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        free(result_data);
        result_data = NULL;
        goto cleanup;
    }
    
    result->data = result_data;
    result->length = result_len;
    result_data = NULL; /* Ownership transferred */
    
cleanup:
    /* Securely clear sensitive data using OPENSSL_cleanse */
    secure_clear(encryption_key, AES_KEY_LENGTH);
    secure_clear(hmac_key, AES_KEY_LENGTH);
    secure_clear(salt, SALT_LENGTH);
    secure_clear(hmac_salt, SALT_LENGTH);
    secure_clear(iv, IV_LENGTH);
    secure_clear(hmac_tag, HMAC_LENGTH);
    
    if (ciphertext != NULL) {
        secure_clear(ciphertext, plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        free(ciphertext);
    }
    if (result_data != NULL) {
        free(result_data);
    }
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    return result;
}

/**
 * Decrypts data encrypted with encrypt_aes_cbc_with_hmac.
 * Returns pointer to EncryptedData on success, NULL on failure.
 * Caller must free the returned structure and its data field.
 */
EncryptedData* decrypt_aes_cbc_with_hmac(const unsigned char *encrypted_data,
                                         size_t encrypted_len,
                                         const char *passphrase) {
    EVP_CIPHER_CTX *ctx = NULL;
    const unsigned char *salt = NULL;
    const unsigned char *hmac_salt = NULL;
    const unsigned char *iv = NULL;
    const unsigned char *ciphertext = NULL;
    const unsigned char *hmac_tag = NULL;
    unsigned char encryption_key[AES_KEY_LENGTH] = {0};
    unsigned char hmac_key[AES_KEY_LENGTH] = {0};
    unsigned char expected_hmac[HMAC_LENGTH] = {0};
    unsigned char *plaintext = NULL;
    EncryptedData *result = NULL;
    size_t offset = 0;
    size_t ciphertext_len = 0;
    size_t min_size = 0;
    int len = 0;
    int plaintext_len = 0;
    unsigned int hmac_len = 0;
    
    /* Input validation */
    if (encrypted_data == NULL || encrypted_len == 0) {
        fprintf(stderr, "Error: Encrypted data must not be empty\\n");
        return NULL;
    }
    if (passphrase == NULL || strlen(passphrase) == 0) {
        fprintf(stderr, "Error: Passphrase must not be empty\\n");
        return NULL;
    }
    
    /* Minimum size check */
    min_size = 4 + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + HMAC_LENGTH;
    if (encrypted_len < min_size) {
        fprintf(stderr, "Error: Invalid encrypted data format\\n");
        return NULL;
    }
    
    /* Parse and validate magic */
    if (memcmp(encrypted_data + offset, MAGIC, 4) != 0) {
        fprintf(stderr, "Error: Invalid file format\\n");
        return NULL;
    }
    offset += 4;
    
    /* Parse and validate version */
    if (encrypted_data[offset] != VERSION) {
        fprintf(stderr, "Error: Unsupported version\\n");
        return NULL;
    }
    offset += 1;
    
    /* Parse salt, hmac_salt, and IV */
    salt = encrypted_data + offset;
    offset += SALT_LENGTH;
    
    hmac_salt = encrypted_data + offset;
    offset += SALT_', type='text')