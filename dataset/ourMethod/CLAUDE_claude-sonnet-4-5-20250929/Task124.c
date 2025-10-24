
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>

/* Constants for cryptographic operations */
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define FILE_MAGIC "ENC1"
#define FILE_VERSION 1
#define MAX_FILE_SIZE (10 * 1024 * 1024)

/* Structure to hold encrypted data */
typedef struct {
    uint8_t* data;
    size_t size;
} EncryptedData;

/* Securely clear sensitive data from memory */
void secure_clear(void* ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        OPENSSL_cleanse(ptr, size);
    }
}

/* Initialize encrypted data structure */
void encrypted_data_init(EncryptedData* ed) {
    if (ed != NULL) {
        ed->data = NULL;
        ed->size = 0;
    }
}

/* Free encrypted data structure */
void encrypted_data_free(EncryptedData* ed) {
    if (ed != NULL && ed->data != NULL) {
        secure_clear(ed->data, ed->size);
        free(ed->data);
        ed->data = NULL;
        ed->size = 0;
    }
}

/* Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256 */
int derive_key(const char* passphrase, size_t passphrase_len,
               const uint8_t* salt, uint8_t* key) {
    if (passphrase == NULL || passphrase_len == 0 || salt == NULL || key == NULL) {
        return 0;
    }
    
    /* Use PBKDF2 with HMAC-SHA-256 for key derivation */
    int result = PKCS5_PBKDF2_HMAC(
        passphrase, (int)passphrase_len,
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE, key
    );
    
    return result == 1;
}

/* Encrypt sensitive data using AES-256-GCM */
int encrypt_data(const char* plaintext, size_t plaintext_len,
                 const char* passphrase, size_t passphrase_len,
                 EncryptedData* output) {
    if (plaintext == NULL || plaintext_len == 0 || passphrase == NULL || 
        passphrase_len == 0 || output == NULL) {
        return 0;
    }
    
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t key[KEY_SIZE];
    uint8_t tag[TAG_SIZE];
    EVP_CIPHER_CTX* ctx = NULL;
    uint8_t* ciphertext = NULL;
    int outlen = 0;
    int total_len = 0;
    int success = 0;
    
    /* Initialize output */
    encrypted_data_init(output);
    
    /* Generate cryptographically secure random salt and IV */
    if (RAND_bytes(salt, SALT_SIZE) != 1 || RAND_bytes(iv, IV_SIZE) != 1) {
        goto cleanup;
    }
    
    /* Derive encryption key from passphrase */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }
    
    /* Create and initialize cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }
    
    /* Initialize AES-256-GCM encryption */
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
    
    /* Allocate ciphertext buffer */
    ciphertext = (uint8_t*)malloc(plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    if (ciphertext == NULL) {
        goto cleanup;
    }
    
    /* Encrypt plaintext */
    if (EVP_EncryptUpdate(ctx, ciphertext, &outlen, 
                          (const uint8_t*)plaintext, (int)plaintext_len) != 1) {
        goto cleanup;
    }
    total_len = outlen;
    
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &outlen) != 1) {
        goto cleanup;
    }
    total_len += outlen;
    
    /* Get authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }
    
    /* Build output: [magic][version][salt][iv][ciphertext][tag] */
    size_t output_size = 4 + 1 + SALT_SIZE + IV_SIZE + total_len + TAG_SIZE;
    output->data = (uint8_t*)malloc(output_size);
    if (output->data == NULL) {
        goto cleanup;
    }
    output->size = output_size;
    
    size_t offset = 0;
    memcpy(output->data + offset, FILE_MAGIC, 4);
    offset += 4;
    output->data[offset] = FILE_VERSION;
    offset += 1;
    memcpy(output->data + offset, salt, SALT_SIZE);
    offset += SALT_SIZE;
    memcpy(output->data + offset, iv, IV_SIZE);
    offset += IV_SIZE;
    memcpy(output->data + offset, ciphertext, total_len);
    offset += total_len;
    memcpy(output->data + offset, tag, TAG_SIZE);
    
    success = 1;
    
cleanup:
    /* Securely clear sensitive data */
    secure_clear(key, KEY_SIZE);
    if (ciphertext != NULL) {
        secure_clear(ciphertext, plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        free(ciphertext);
    }
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    if (!success && output->data != NULL) {
        encrypted_data_free(output);
    }
    
    return success;
}

/* Decrypt sensitive data using AES-256-GCM */
int decrypt_data(const EncryptedData* input, const char* passphrase, 
                 size_t passphrase_len, char** plaintext, size_t* plaintext_len) {
    if (input == NULL || input->data == NULL || passphrase == NULL || 
        passphrase_len == 0 || plaintext == NULL || plaintext_len == NULL) {
        return 0;
    }
    
    /* Validate minimum size: magic(4) + version(1) + salt(16) + iv(12) + tag(16) = 49 */
    if (input->size < 49) {
        return 0;
    }
    
    /* Validate magic and version */
    if (memcmp(input->data, FILE_MAGIC, 4) != 0 || input->data[4] != FILE_VERSION) {
        return 0;
    }
    
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t key[KEY_SIZE];
    uint8_t tag[TAG_SIZE];
    EVP_CIPHER_CTX* ctx = NULL;
    uint8_t* plaintext_buf = NULL;
    int outlen = 0;
    int total_len = 0;
    int success = 0;
    
    /* Extract components */
    size_t offset = 5;
    memcpy(salt, input->data + offset, SALT_SIZE);
    offset += SALT_SIZE;
    memcpy(iv, input->data + offset, IV_SIZE);
    offset += IV_SIZE;
    
    size_t ciphertext_len = input->size - offset - TAG_SIZE;
    const uint8_t* ciphertext = input->data + offset;
    memcpy(tag, input->data + input->size - TAG_SIZE, TAG_SIZE);
    
    /* Derive key */
    if (!derive_key(passphrase, passphrase_len, salt, key)) {
        goto cleanup;
    }
    
    /* Create and initialize cipher context */
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
    
    /* Allocate plaintext buffer */
    plaintext_buf = (uint8_t*)malloc(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()) + 1);
    if (plaintext_buf == NULL) {
        goto cleanup;
    }
    
    /* Decrypt ciphertext */
    if (EVP_DecryptUpdate(ctx, plaintext_buf, &outlen, ciphertext, (int)ciphertext_len) != 1) {
        goto cleanup;
    }
    total_len = outlen;
    
    /* Set expected authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }
    
    /* Finalize decryption and verify tag */
    int ret = EVP_DecryptFinal_ex(ctx, plaintext_buf + outlen, &outlen);
    if (ret != 1) {
        /* Tag verification failed - do not reveal partial plaintext */
        goto cleanup;
    }
    total_len += outlen;
    
    /* Null-terminate and assign output */
    plaintext_buf[total_len] = '\\0';
    *plaintext = (char*)plaintext_buf;
    *plaintext_len = total_len;
    plaintext_buf = NULL; /* Transfer ownership */
    
    success = 1;
    
cleanup:
    /* Securely clear sensitive data */
    secure_clear(key, KEY_SIZE);
    if (plaintext_buf != NULL) {
        secure_clear(plaintext_buf, ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()) + 1);
        free(plaintext_buf);
    }
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    return success;
}

/* Save encrypted data to file with atomic write */
int save_to_file(const char* filename, const EncryptedData* data) {
    if (filename == NULL || data == NULL || data->data == NULL || data->size == 0) {
        return 0;
    }
    
    /* Create temporary filename */
    size_t temp_len = strlen(filename) + 5;
    char* temp_filename = (char*)malloc(temp_len);
    if (temp_filename == NULL) {
        return 0;
    }
    
    int snprintf_result = snprintf(temp_filename, temp_len, "%s.tmp", filename);
    if (snprintf_result < 0 || (size_t)snprintf_result >= temp_len) {
        free(temp_filename);
        return 0;
    }
    
    /* Write to temporary file */
    FILE* file = fopen(temp_filename, "wb");
    if (file == NULL) {
        free(temp_filename);
        return 0;
    }
    
    size_t written = fwrite(data->data, 1, data->size, file);
    fflush(file);
    int flush_result = ferror(file);
    fclose(file);
    
    if (written != data->size || flush_result) {
        remove(temp_filename);
        free(temp_filename);
        return 0;
    }
    
    /* Atomic rename */
    if (rename(temp_filename, filename) != 0) {
        remove(temp_filename);
        free(temp_filename);
        return 0;
    }
    
    free(temp_filename);
    return 1;
}

/* Load encrypted data from file */
int load_from_file(const char* filename, EncryptedData* data) {
    if (filename == NULL || data == NULL) {
        return 0;
    }
    
    encrypted_data_init(data);
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return 0;
    }
    
    /* Get file size */
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    
    long file_size = ftell(file);
    if (file_size <= 0 || file_size > MAX_FILE_SIZE) {
        fclose(file);
        return 0;
    }
    
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    
    /* Allocate and read data */
    data->data = (uint8_t*)malloc((size_t)file_size);
    if (data->data == NULL) {
        fclose(file);
        return 0;
    }
    data->size = (size_t)file_size;
    
    size_t read_size = fread(data->data, 1, data->size, file);
    fclose(file);
    
    if (read_size != data->size) {
        encrypted_data_free(data);
        return 0;
    }
    
    return 1;
}

int main(void) {
    /* Test case 1: Encrypt and decrypt credit card number */
    {
        const char* sensitive_data = "4532-1234-5678-9010";
        const char* passphrase = "StrongPassphrase123!@#";
        EncryptedData encrypted;
        encrypted_data_init(&encrypted);
        
        if (encrypt_data(sensitive_data, strlen(sensitive_data), 
                        passphrase, strlen(passphrase), &encrypted)) {
            char* decrypted = NULL;
            size_t decrypted_len = 0;
            if (decrypt_data(&encrypted, passphrase, strlen(passphrase), 
                           &decrypted, &decrypted_len)) {
                printf("Test 1 PASSED: Credit card encryption/decryption successful\\n");
                secure_clear(decrypted, decrypted_len);
                free(decrypted);
            } else {
                printf("Test 1 FAILED: Decryption failed\\n");
            }
        } else {
            printf("Test 1 FAILED: Encryption failed\\n");
        }
        encrypted_data_free(&encrypted);
    }
    
    /* Test case 2: Encrypt personal information and save to file */
    {
        const char* personal_info = "SSN:123-45-6789|DOB:1990-01-01|Name:John Doe";
        const char* passphrase = "SecurePassword456$%^";
        EncryptedData encrypted;
        encrypted_data_init(&encrypted);
        
        if (encrypt_data(personal_info, strlen(personal_info),
                        passphrase, strlen(passphrase), &encrypted)) {
            if (save_to_file("personal_data.enc", &encrypted)) {
                EncryptedData loaded;
                encrypted_data_init(&loaded);
                if (load_from_file("personal_data.enc", &loaded)) {
                    char* decrypted = NULL;
                    size_t decrypted_len = 0;
                    if (decrypt_data(&loaded, passphrase, strlen(passphrase),
                                   &decrypted, &decrypted_len)) {
                        printf("Test 2 PASSED: File save/load and decryption successful\\n");
                        secure_clear(decrypted, decrypted_len);
                        free(decrypted);
                    } else {
                        printf("Test 2 FAILED: Decryption failed\\n");
                    }
                    encrypted_data_free(&loaded);
                } else {
                    printf("Test 2 FAILED: File load failed\\n");
                }
                remove("personal_data.enc");
            } else {
                printf("Test 2 FAILED: File save failed\\n");
            }
        } else {
            printf("Test 2 FAILED: Encryption failed\\n");
        }
        encrypted_data_free(&encrypted);
    }
    
    /* Test case 3: Wrong passphrase should fail */
    {
        const char* data = "Secret Information";
        const char* correct_pass = "CorrectPassword789";
        const char* wrong_pass = "WrongPassword789";
        EncryptedData encrypted;
        encrypted_data_init(&encrypted);
        
        if (encrypt_data(data, strlen(data), correct_pass, strlen(correct_pass), &encrypted)) {
            char* decrypted = NULL;
            size_t decrypted_len = 0;
            if (!decrypt_data(&encrypted, wrong_pass, strlen(wrong_pass), 
                            &decrypted, &decrypted_len)) {
                printf("Test 3 PASSED: Wrong passphrase correctly rejected\\n");
            } else {
                printf("Test 3 FAILED: Wrong passphrase accepted\\n");
                if (decrypted != NULL) {
                    secure_clear(decrypted, decrypted_len);
                    free(decrypted);
                }
            }
        } else {
            printf("Test 3 FAILED: Encryption failed\\n");
        }
        encrypted_data_free(&encrypted);
    }
    
    /* Test case 4: Empty data handling */
    {
        const char* empty_data = "";
        const char* passphrase = "TestPassword";
        EncryptedData encrypted;
        encrypted_data_init(&encrypted);
        
        if (!encrypt_data(empty_data, 0, passphrase, strlen(passphrase), &encrypted)) {
            printf("Test 4 PASSED: Empty data correctly rejected\\n");
        } else {
            printf("Test 4 FAILED: Empty data accepted\\n");
            encrypted_data_free(&encrypted);
        }
    }
    
    /* Test case 5: Multiple encryptions produce different ciphertexts */
    {
        const char* data = "Test Data";
        const char* passphrase = "SamePassphrase";
        EncryptedData encrypted1, encrypted2;
        encrypted_data_init(&encrypted1);
        encrypted_data_init(&encrypted2);
        
        if (encrypt_data(data, strlen(data), passphrase, strlen(passphrase), &encrypted1) &&
            encrypt_data(data, strlen(data), passphrase, strlen(passphrase), &encrypted2)) {
            if (encrypted1.size == encrypted2.size && 
                memcmp(encrypted1.data, encrypted2.data, encrypted1.size) != 0) {
                printf("Test 5 PASSED: Unique IVs generate different ciphertexts\\n");
            } else {
                printf("Test 5 FAILED: Same ciphertext produced\\n");
            }
        } else {
            printf("Test 5 FAILED: Encryption failed\\n");
        }
        encrypted_data_free(&encrypted1);
        encrypted_data_free(&encrypted2);
    }
    
    return 0;
}
