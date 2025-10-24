
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* File format: [magic(4)][version(1)][salt(16)][iv(12)][ciphertext][tag(16)] */
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MAGIC "ENC1"
#define VERSION 1
#define MAX_PLAINTEXT_SIZE 1073741824 /* 1GB limit */

/* Structure to hold encrypted data */
typedef struct {
    unsigned char *data;
    size_t length;
} EncryptedData;

/* Securely clear memory before deallocation */
static void secure_clear(void *ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        volatile unsigned char *p = (volatile unsigned char *)ptr;
        size_t i;
        for (i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
}

/* Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256 */
static int derive_key(const char *passphrase, size_t passphrase_len,
                     const unsigned char *salt, unsigned char *key) {
    /* Validate inputs - all pointers initialized or checked */
    if (passphrase == NULL || salt == NULL || key == NULL || passphrase_len == 0) {
        return 0;
    }

    /* Use PBKDF2-HMAC-SHA-256 with 210,000 iterations for key derivation */
    int result = PKCS5_PBKDF2_HMAC(
        passphrase,
        (int)passphrase_len,
        salt,
        SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE,
        key
    );

    return result == 1;
}

/* Encrypt plaintext using AES-256-GCM */
EncryptedData* encrypt_string(const char *plaintext, size_t plaintext_len,
                               const char *passphrase, size_t passphrase_len) {
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE];
    unsigned char tag[TAG_SIZE];
    EVP_CIPHER_CTX *ctx = NULL;
    EncryptedData *result = NULL;
    unsigned char *ciphertext = NULL;
    int len = 0;
    int ciphertext_total_len = 0;
    size_t output_size = 0;
    size_t ciphertext_max_len = 0;

    /* Initialize all sensitive buffers to zero */
    memset(salt, 0, SALT_SIZE);
    memset(iv, 0, IV_SIZE);
    memset(key, 0, KEY_SIZE);
    memset(tag, 0, TAG_SIZE);

    /* Validate inputs - treat all inputs as untrusted */
    if (plaintext == NULL || passphrase == NULL) {
        return NULL;
    }
    if (plaintext_len == 0 || plaintext_len > MAX_PLAINTEXT_SIZE) {
        return NULL;
    }
    if (passphrase_len == 0) {
        return NULL;
    }

    /* Generate cryptographically secure random salt (unique per encryption) */
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        goto cleanup;
    }

    /* Generate cryptographically secure random IV (unique per encryption, never reuse) */
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        goto cleanup;
    }

    /* Derive encryption key from passphrase using PBKDF2 */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }

    /* Create cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }

    /* Initialize AES-256-GCM encryption */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        goto cleanup;
    }

    /* Set IV length (12 bytes for GCM) */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) {
        goto cleanup;
    }

    /* Set key and IV */
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        goto cleanup;
    }

    /* Allocate ciphertext buffer with bounds checking */
    ciphertext_max_len = plaintext_len + EVP_CIPHER_CTX_block_size(ctx);
    
    /* Check for integer overflow */
    if (ciphertext_max_len < plaintext_len) {
        goto cleanup;
    }

    ciphertext = (unsigned char *)malloc(ciphertext_max_len);
    if (ciphertext == NULL) {
        goto cleanup;
    }

    /* Perform encryption */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len,
                         (const unsigned char *)plaintext,
                         (int)plaintext_len) != 1) {
        goto cleanup;
    }
    ciphertext_total_len = len;

    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        goto cleanup;
    }
    ciphertext_total_len += len;

    /* Get the 16-byte authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }

    /* Calculate total output size with overflow check */
    output_size = 4 + 1 + SALT_SIZE + IV_SIZE + ciphertext_total_len + TAG_SIZE;
    if (output_size < (size_t)ciphertext_total_len) {
        goto cleanup;
    }

    /* Allocate result structure */
    result = (EncryptedData *)malloc(sizeof(EncryptedData));
    if (result == NULL) {
        goto cleanup;
    }

    result->data = (unsigned char *)malloc(output_size);
    if (result->data == NULL) {
        free(result);
        result = NULL;
        goto cleanup;
    }

    result->length = output_size;

    /* Build output: [magic][version][salt][iv][ciphertext][tag] */
    size_t pos = 0;
    
    /* Copy magic bytes */
    memcpy(result->data + pos, MAGIC, 4);
    pos += 4;
    
    /* Copy version */
    result->data[pos] = VERSION;
    pos += 1;
    
    /* Copy salt */
    memcpy(result->data + pos, salt, SALT_SIZE);
    pos += SALT_SIZE;
    
    /* Copy IV */
    memcpy(result->data + pos, iv, IV_SIZE);
    pos += IV_SIZE;
    
    /* Copy ciphertext */
    memcpy(result->data + pos, ciphertext, ciphertext_total_len);
    pos += ciphertext_total_len;
    
    /* Copy authentication tag */
    memcpy(result->data + pos, tag, TAG_SIZE);

cleanup:
    /* Securely clear all sensitive data */
    secure_clear(salt, SALT_SIZE);
    secure_clear(iv, IV_SIZE);
    secure_clear(key, KEY_SIZE);
    secure_clear(tag, TAG_SIZE);
    
    if (ciphertext != NULL) {
        secure_clear(ciphertext, ciphertext_max_len);
        free(ciphertext);
    }
    
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }

    return result;
}

/* Decrypt ciphertext using AES-256-GCM */
char* decrypt_string(const EncryptedData *encrypted_data,
                    const char *passphrase, size_t passphrase_len,
                    size_t *plaintext_len_out) {
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE];
    unsigned char tag[TAG_SIZE];
    EVP_CIPHER_CTX *ctx = NULL;
    char *plaintext = NULL;
    unsigned char *ciphertext_ptr = NULL;
    size_t ciphertext_len = 0;
    size_t pos = 0;
    int len = 0;
    int plaintext_total_len = 0;
    int ret = 0;
    const size_t MIN_SIZE = 4 + 1 + SALT_SIZE + IV_SIZE + TAG_SIZE;

    /* Initialize sensitive buffers */
    memset(salt, 0, SALT_SIZE);
    memset(iv, 0, IV_SIZE);
    memset(key, 0, KEY_SIZE);
    memset(tag, 0, TAG_SIZE);

    /* Validate inputs */
    if (encrypted_data == NULL || encrypted_data->data == NULL || 
        passphrase == NULL || plaintext_len_out == NULL) {
        return NULL;
    }

    if (passphrase_len == 0) {
        return NULL;
    }

    /* Validate minimum size */
    if (encrypted_data->length < MIN_SIZE) {
        return NULL;
    }

    /* Validate magic bytes */
    if (memcmp(encrypted_data->data, MAGIC, 4) != 0) {
        return NULL;
    }
    pos += 4;

    /* Validate version */
    if (encrypted_data->data[pos] != VERSION) {
        return NULL;
    }
    pos += 1;

    /* Extract salt */
    memcpy(salt, encrypted_data->data + pos, SALT_SIZE);
    pos += SALT_SIZE;

    /* Extract IV */
    memcpy(iv, encrypted_data->data + pos, IV_SIZE);
    pos += IV_SIZE;

    /* Calculate ciphertext length */
    ciphertext_len = encrypted_data->length - pos - TAG_SIZE;
    if (ciphertext_len > MAX_PLAINTEXT_SIZE) {
        goto cleanup;
    }

    ciphertext_ptr = encrypted_data->data + pos;

    /* Extract tag (last 16 bytes) */
    memcpy(tag, encrypted_data->data + encrypted_data->length - TAG_SIZE, TAG_SIZE);

    /* Derive key from passphrase */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }

    /* Create cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }

    /* Initialize AES-256-GCM decryption */
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        goto cleanup;
    }

    /* Set IV length */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) {
        goto cleanup;
    }

    /* Set key and IV */
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        goto cleanup;
    }

    /* Allocate plaintext buffer with bounds check */
    plaintext = (char *)malloc(ciphertext_len + EVP_CIPHER_CTX_block_size(ctx) + 1);
    if (plaintext == NULL) {
        goto cleanup;
    }

    /* Initialize plaintext buffer */
    memset(plaintext, 0, ciphertext_len + EVP_CIPHER_CTX_block_size(ctx) + 1);

    /* Decrypt */
    if (EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &len,
                         ciphertext_ptr, (int)ciphertext_len) != 1) {
        goto cleanup;
    }
    plaintext_total_len = len;

    /* Set expected tag value for verification */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }

    /* Finalize and verify tag - fail closed on verification failure */
    ret = EVP_DecryptFinal_ex(ctx, (unsigned char *)plaintext + len, &len);
    if (ret <= 0) {
        /* Authentication failed - clear plaintext and fail */
        if (plaintext != NULL) {
            secure_clear(plaintext, ciphertext_len + EVP_CIPHER_CTX_block_size(ctx));
            free(plaintext);
            plaintext = NULL;
        }
        goto cleanup;
    }

    plaintext_total_len += len;
    plaintext[plaintext_total_len] = '\\0'; /* Null-terminate string */
    *plaintext_len_out = plaintext_total_len;

cleanup:
    /* Securely clear all sensitive data */
    secure_clear(salt, SALT_SIZE);
    secure_clear(iv, IV_SIZE);
    secure_clear(key, KEY_SIZE);
    secure_clear(tag, TAG_SIZE);
    
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }

    return plaintext;
}

/* Free encrypted data structure */
void free_encrypted_data(EncryptedData *data) {
    if (data != NULL) {
        if (data->data != NULL) {
            secure_clear(data->data, data->length);
            free(data->data);
        }
        free(data);
    }
}

int main(void) {
    int all_tests_passed = 1;

    /* Test case 1: Basic encryption and decryption */
    {
        const char *plaintext = "Hello, World!";
        const char *passphrase = "SecurePassword123!";
        size_t plaintext_len = strlen(plaintext);
        size_t passphrase_len = strlen(passphrase);
        size_t decrypted_len = 0;
        
        EncryptedData *encrypted = encrypt_string(plaintext, plaintext_len,
                                                  passphrase, passphrase_len);
        if (encrypted == NULL) {
            printf("Test 1: FAIL (encryption failed)\\n");
            all_tests_passed = 0;
        } else {
            char *decrypted = decrypt_string(encrypted, passphrase, passphrase_len, &decrypted_len);
            
            if (decrypted == NULL || decrypted_len != plaintext_len ||
                memcmp(plaintext, decrypted, plaintext_len) != 0) {
                printf("Test 1: FAIL\\n");
                all_tests_passed = 0;
            } else {
                printf("Test 1: PASS\\n");
            }
            
            if (decrypted != NULL) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }

    /* Test case 2: Longer text */
    {
        const char *plaintext = "This is a longer text to test the encryption algorithm with multiple blocks.";
        const char *passphrase = "AnotherSecurePass456@";
        size_t plaintext_len = strlen(plaintext);
        size_t passphrase_len = strlen(passphrase);
        size_t decrypted_len = 0;
        
        EncryptedData *encrypted = encrypt_string(plaintext, plaintext_len,
                                                  passphrase, passphrase_len);
        if (encrypted == NULL) {
            printf("Test 2: FAIL (encryption failed)\\n");
            all_tests_passed = 0;
        } else {
            char *decrypted = decrypt_string(encrypted, passphrase, passphrase_len, &decrypted_len);
            
            if (decrypted == NULL || decrypted_len != plaintext_len ||
                memcmp(plaintext, decrypted, plaintext_len) != 0) {
                printf("Test 2: FAIL\\n");
                all_tests_passed = 0;
            } else {
                printf("Test 2: PASS\\n");
            }
            
            if (decrypted != NULL) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }

    /* Test case 3: Special characters */
    {
        const char *plaintext = "Special chars: !@#$%^&*()_+-=[]{}|;:',.<>?/~`";
        const char *passphrase = "ComplexPass789#";
        size_t plaintext_len = strlen(plaintext);
        size_t passphrase_len = strlen(passphrase);
        size_t decrypted_len = 0;
        
        EncryptedData *encrypted = encrypt_string(plaintext, plaintext_len,
                                                  passphrase, passphrase_len);
        if (encrypted == NULL) {
            printf("Test 3: FAIL (encryption failed)\\n");
            all_tests_passed = 0;
        } else {
            char *decrypted = decrypt_string(encrypted, passphrase, passphrase_len, &decrypted_len);
            
            if (decrypted == NULL || decrypted_len != plaintext_len ||
                memcmp(plaintext, decrypted, plaintext_len) != 0) {
                printf("Test 3: FAIL\\n");
                all_tests_passed = 0;
            } else {
                printf("Test 3: PASS\\n");
            }
            
            if (decrypted != NULL) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }

    /* Test case 4: Unicode characters */
    {
        const char *plaintext = "Unicode test: こんにちは世界";
        const char *passphrase = "UnicodeTest2024!";
        size_t plaintext_len = strlen(plaintext);
        size_t passphrase_len = strlen(passphrase);
        size_t decrypted_len = 0;
        
        EncryptedData *encrypted = encrypt_string(plaintext, plaintext_len,
                                                  passphrase, passphrase_len);
        if (encrypted == NULL) {
            printf("Test 4: FAIL (encryption failed)\\n");
            all_tests_passed = 0;
        } else {
            char *decrypted = decrypt_string(encrypted, passphrase, passphrase_len, &decrypted_len);
            
            if (decrypted == NULL || decrypted_len != plaintext_len ||
                memcmp(plaintext, decrypted, plaintext_len) != 0) {
                printf("Test 4: FAIL\\n");
                all_tests_passed = 0;
            } else {
                printf("Test 4: PASS\\n");
            }
            
            if (decrypted != NULL) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }

    /* Test case 5: Wrong passphrase (should fail) */
    {
        const char *plaintext = "Secret message";
        const char *passphrase1 = "CorrectPassword!";
        const char *passphrase2 = "WrongPassword!";
        size_t plaintext_len = strlen(plaintext);
        size_t passphrase1_len = strlen(passphrase1);
        size_t passphrase2_len = strlen(passphrase2);
        size_t decrypted_len = 0;
        
        EncryptedData *encrypted = encrypt_string(plaintext, plaintext_len,
                                                  passphrase1, passphrase1_len);
        if (encrypted == NULL) {
            printf("Test 5: FAIL (encryption failed)\\n");
            all_tests_passed = 0;
        } else {
            char *decrypted = decrypt_string(encrypted, passphrase2, passphrase2_len, &decrypted_len);
            
            /* Should fail with wrong password */
            if (decrypted != NULL) {
                printf("Test 5: FAIL (should have failed with wrong password)\\n");
                all_tests_passed = 0;
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            } else {
                printf("Test 5: PASS (wrong password detection)\\n");
            }
            
            free_encrypted_data(encrypted);
        }
    }

    return all_tests_passed ? 0 : 1;
}
