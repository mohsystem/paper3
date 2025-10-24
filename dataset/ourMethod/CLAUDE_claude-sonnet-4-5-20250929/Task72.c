
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define PBKDF2_ITERATIONS 210000

/* Structure to hold encrypted data with metadata */
typedef struct {
    unsigned char* data;
    size_t length;
} EncryptedData;

/* Generate cryptographically secure random bytes per Rules#5 and #6 */
static int generate_random(unsigned char* buffer, size_t size) {
    if (!buffer || size == 0) {
        return 0;
    }
    
    /* Use OpenSSL CSPRNG */
    if (RAND_bytes(buffer, size) != 1) {
        return 0;
    }
    
    return 1;
}

/* Derive key from passphrase using PBKDF2-HMAC-SHA256 per Rules#5 */
static int derive_key(const char* passphrase, size_t passphrase_len,
                     const unsigned char* salt, unsigned char* key) {
    if (!passphrase || passphrase_len == 0 || !salt || !key) {
        return 0;
    }
    
    /* Use PBKDF2 for key derivation */
    if (PKCS5_PBKDF2_HMAC(passphrase, passphrase_len,
                          salt, SALT_SIZE,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          KEY_SIZE, key) != 1) {
        return 0;
    }
    
    return 1;
}

/* Securely clear memory per Rules#1 */
static void secure_clear(void* ptr, size_t len) {
    if (ptr && len > 0) {
        OPENSSL_cleanse(ptr, len);
    }
}

/* Encrypt data using AES-256-GCM with unique IV per Rules#5 and #6 */
EncryptedData* encrypt_data(const char* plaintext, size_t plaintext_len,
                            const char* passphrase, size_t passphrase_len) {
    /* Validate inputs per Rules#1 */
    if (!plaintext || plaintext_len == 0 || !passphrase || passphrase_len == 0) {
        return NULL;
    }
    
    EncryptedData* result = NULL;
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE];
    unsigned char tag[TAG_SIZE];
    EVP_CIPHER_CTX* ctx = NULL;
    unsigned char* ciphertext = NULL;
    int len = 0;
    int ciphertext_len = 0;
    
    /* Initialize all pointers per Rules#1 C checklist */
    memset(salt, 0, SALT_SIZE);
    memset(iv, 0, IV_SIZE);
    memset(key, 0, KEY_SIZE);
    memset(tag, 0, TAG_SIZE);
    
    /* Generate unique salt and IV per encryption per Rules#5 */
    if (!generate_random(salt, SALT_SIZE)) {
        goto cleanup;
    }
    
    if (!generate_random(iv, IV_SIZE)) {
        goto cleanup;
    }
    
    /* Derive encryption key */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }
    
    /* Initialize cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        goto cleanup;
    }
    
    /* Initialize AES-256-GCM encryption per Rules#5 and #6 */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        goto cleanup;
    }
    
    /* Set IV length */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) {
        goto cleanup;
    }
    
    /* Set key and IV */
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        goto cleanup;
    }
    
    /* Allocate ciphertext buffer with bounds check per Rules#1 C checklist */
    size_t max_ciphertext_len = plaintext_len + EVP_CIPHER_CTX_block_size(ctx);
    if (max_ciphertext_len < plaintext_len) {
        goto cleanup;
    }
    
    ciphertext = malloc(max_ciphertext_len);
    if (!ciphertext) {
        goto cleanup;
    }
    
    /* Encrypt plaintext */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len,
                         (const unsigned char*)plaintext, plaintext_len) != 1) {
        goto cleanup;
    }
    ciphertext_len = len;
    
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        goto cleanup;
    }
    ciphertext_len += len;
    
    /* Get authentication tag per Rules#5 */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }
    
    /* Build output: [salt][iv][ciphertext][tag] */
    size_t total_len = SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE;
    
    /* Check for integer overflow per Rules#1 C checklist */
    if (total_len < ciphertext_len) {
        goto cleanup;
    }
    
    result = malloc(sizeof(EncryptedData));
    if (!result) {
        goto cleanup;
    }
    
    result->data = malloc(total_len);
    if (!result->data) {
        free(result);
        result = NULL;
        goto cleanup;
    }
    
    result->length = total_len;
    
    /* Copy components with bounds checking per Rules#1 C checklist */
    memcpy(result->data, salt, SALT_SIZE);
    memcpy(result->data + SALT_SIZE, iv, IV_SIZE);
    memcpy(result->data + SALT_SIZE + IV_SIZE, ciphertext, ciphertext_len);
    memcpy(result->data + SALT_SIZE + IV_SIZE + ciphertext_len, tag, TAG_SIZE);
    
cleanup:
    /* Clear sensitive data per Rules#1 */
    secure_clear(key, KEY_SIZE);
    secure_clear(tag, TAG_SIZE);
    
    if (ciphertext) {
        secure_clear(ciphertext, max_ciphertext_len);
        free(ciphertext);
    }
    
    if (ctx) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    return result;
}

/* Decrypt data encrypted with AES-256-GCM */
char* decrypt_data(const unsigned char* encrypted, size_t encrypted_len,
                   const char* passphrase, size_t passphrase_len,
                   size_t* plaintext_len_out) {
    /* Validate inputs per Rules#1 */
    if (!encrypted || !passphrase || passphrase_len == 0 || !plaintext_len_out) {
        return NULL;
    }
    
    /* Validate minimum size per Rules#1 C checklist */
    if (encrypted_len < SALT_SIZE + IV_SIZE + TAG_SIZE) {
        return NULL;
    }
    
    unsigned char key[KEY_SIZE];
    EVP_CIPHER_CTX* ctx = NULL;
    char* plaintext = NULL;
    int len = 0;
    int plaintext_len = 0;
    
    /* Initialize key per Rules#1 C checklist */
    memset(key, 0, KEY_SIZE);
    
    /* Extract components with bounds checking */
    const unsigned char* salt = encrypted;
    const unsigned char* iv = encrypted + SALT_SIZE;
    size_t ciphertext_len = encrypted_len - SALT_SIZE - IV_SIZE - TAG_SIZE;
    const unsigned char* ciphertext = encrypted + SALT_SIZE + IV_SIZE;
    const unsigned char* tag = encrypted + encrypted_len - TAG_SIZE;
    
    /* Derive decryption key */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }
    
    /* Initialize cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
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
    
    /* Allocate plaintext buffer with bounds check per Rules#1 C checklist */
    plaintext = malloc(ciphertext_len + 1);
    if (!plaintext) {
        goto cleanup;
    }
    
    /* Null-terminate for safety per Rules#1 C checklist */
    plaintext[ciphertext_len] = '\\0';
    
    /* Decrypt ciphertext */
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &len,
                         ciphertext, ciphertext_len) != 1) {
        goto cleanup;
    }
    plaintext_len = len;
    
    /* Set expected tag per Rules#5 */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE,
                           (void*)tag) != 1) {
        goto cleanup;
    }
    
    /* Finalize and verify tag - fail closed on mismatch per Rules#5 */
    int ret = EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + len, &len);
    
    if (ret != 1) {
        /* Authentication failed - clear and free plaintext */
        secure_clear(plaintext, ciphertext_len);
        free(plaintext);
        plaintext = NULL;
        goto cleanup;
    }
    
    plaintext_len += len;
    plaintext[plaintext_len] = '\\0';
    *plaintext_len_out = plaintext_len;
    
cleanup:
    /* Clear sensitive data per Rules#1 */
    secure_clear(key, KEY_SIZE);
    
    if (ctx) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    return plaintext;
}

/* Free encrypted data structure */
void free_encrypted_data(EncryptedData* data) {
    if (data) {
        if (data->data) {
            secure_clear(data->data, data->length);
            free(data->data);
        }
        free(data);
    }
}

int main(void) {
    int all_pass = 1;
    
    /* Test case 1: Basic encryption/decryption */
    {
        const char* plaintext = "Hello, World!";
        const char* passphrase = "SecurePassphrase123!";
        
        EncryptedData* encrypted = encrypt_data(plaintext, strlen(plaintext),
                                               passphrase, strlen(passphrase));
        if (!encrypted) {
            printf("Test 1: FAIL (encryption failed)\\n");
            all_pass = 0;
        } else {
            size_t decrypted_len = 0;
            char* decrypted = decrypt_data(encrypted->data, encrypted->length,
                                          passphrase, strlen(passphrase),
                                          &decrypted_len);
            
            if (decrypted && decrypted_len == strlen(plaintext) &&
                memcmp(decrypted, plaintext, strlen(plaintext)) == 0) {
                printf("Test 1: PASS\\n");
            } else {
                printf("Test 1: FAIL\\n");
                all_pass = 0;
            }
            
            if (decrypted) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }
    
    /* Test case 2: Longer text */
    {
        const char* plaintext = "This is a longer message to test encryption with multiple blocks.";
        const char* passphrase = "AnotherSecureKey456$";
        
        EncryptedData* encrypted = encrypt_data(plaintext, strlen(plaintext),
                                               passphrase, strlen(passphrase));
        if (!encrypted) {
            printf("Test 2: FAIL (encryption failed)\\n");
            all_pass = 0;
        } else {
            size_t decrypted_len = 0;
            char* decrypted = decrypt_data(encrypted->data, encrypted->length,
                                          passphrase, strlen(passphrase),
                                          &decrypted_len);
            
            if (decrypted && decrypted_len == strlen(plaintext) &&
                memcmp(decrypted, plaintext, strlen(plaintext)) == 0) {
                printf("Test 2: PASS\\n");
            } else {
                printf("Test 2: FAIL\\n");
                all_pass = 0;
            }
            
            if (decrypted) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }
    
    /* Test case 3: Special characters */
    {
        const char* plaintext = "Special chars: !@#$%^&*()";
        const char* passphrase = "ComplexPass789&*";
        
        EncryptedData* encrypted = encrypt_data(plaintext, strlen(plaintext),
                                               passphrase, strlen(passphrase));
        if (!encrypted) {
            printf("Test 3: FAIL (encryption failed)\\n");
            all_pass = 0;
        } else {
            size_t decrypted_len = 0;
            char* decrypted = decrypt_data(encrypted->data, encrypted->length,
                                          passphrase, strlen(passphrase),
                                          &decrypted_len);
            
            if (decrypted && decrypted_len == strlen(plaintext) &&
                memcmp(decrypted, plaintext, strlen(plaintext)) == 0) {
                printf("Test 3: PASS\\n");
            } else {
                printf("Test 3: FAIL\\n");
                all_pass = 0;
            }
            
            if (decrypted) {
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            free_encrypted_data(encrypted);
        }
    }
    
    /* Test case 4: Verify unique IVs produce different ciphertexts */
    {
        const char* plaintext = "Hello, World!";
        const char* passphrase = "SecurePassphrase123!";
        
        EncryptedData* encrypted1 = encrypt_data(plaintext, strlen(plaintext),
                                                passphrase, strlen(passphrase));
        EncryptedData* encrypted2 = encrypt_data(plaintext, strlen(plaintext),
                                                passphrase, strlen(passphrase));
        
        if (!encrypted1 || !encrypted2) {
            printf("Test 4: FAIL (encryption failed)\\n");
            all_pass = 0;
        } else if (encrypted1->length == encrypted2->length &&
                   memcmp(encrypted1->data, encrypted2->data, encrypted1->length) == 0) {
            printf("Test 4: FAIL (IVs not unique)\\n");
            all_pass = 0;
        } else {
            printf("Test 4: PASS\\n");
        }
        
        free_encrypted_data(encrypted1);
        free_encrypted_data(encrypted2);
    }
    
    /* Test case 5: Wrong passphrase should fail */
    {
        const char* plaintext = "Hello, World!";
        const char* passphrase = "SecurePassphrase123!";
        const char* wrong_passphrase = "WrongPassphrase";
        
        EncryptedData* encrypted = encrypt_data(plaintext, strlen(plaintext),
                                               passphrase, strlen(passphrase));
        if (!encrypted) {
            printf("Test 5: FAIL (encryption failed)\\n");
            all_pass = 0;
        } else {
            size_t decrypted_len = 0;
            char* decrypted = decrypt_data(encrypted->data, encrypted->length,
                                          wrong_passphrase, strlen(wrong_passphrase),
                                          &decrypted_len);
            
            if (decrypted == NULL) {
                printf("Test 5: PASS (correctly rejected wrong passphrase)\\n");
            } else {
                printf("Test 5: FAIL (should have rejected wrong passphrase)\\n");
                all_pass = 0;
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            }
            
            free_encrypted_data(encrypted);
        }
    }
    
    return all_pass ? 0 : 1;
}
