
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

/* File format constants */
#define KEY_MAGIC "KEY1"
#define KEY_VERSION 1
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MAX_KEY_DATA_SIZE (1024 * 1024)
#define HEADER_SIZE 8

/* Securely clear memory */
void secure_clear(void* ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        volatile unsigned char* p = (volatile unsigned char*)ptr;
        size_t i;
        for (i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
}

/* Derive key from passphrase using PBKDF2-HMAC-SHA-256 */
int derive_key(const char* passphrase, const unsigned char* salt,
               size_t salt_size, unsigned char* key, size_t key_size) {
    if (passphrase == NULL || salt == NULL || key == NULL ||
        salt_size != SALT_SIZE || key_size != KEY_SIZE) {
        return 0;
    }
    
    int result = PKCS5_PBKDF2_HMAC(
        passphrase, (int)strlen(passphrase),
        salt, (int)salt_size,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        (int)key_size, key
    );
    
    return result == 1;
}

/* Encrypt key data using AES-256-GCM */
int encrypt_key_data(const char* passphrase, const unsigned char* plaintext,
                     size_t plaintext_len, unsigned char** output, size_t* output_len) {
    EVP_CIPHER_CTX* ctx = NULL;
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE];
    unsigned char* result = NULL;
    int len = 0;
    int ciphertext_len = 0;
    size_t total_size;
    int success = 0;
    
    /* Validate inputs */
    if (passphrase == NULL || plaintext == NULL || output == NULL || 
        output_len == NULL || plaintext_len == 0 || plaintext_len > MAX_KEY_DATA_SIZE) {
        return 0;
    }
    
    /* Initialize output to NULL */
    *output = NULL;
    *output_len = 0;
    
    /* Generate random salt and IV */
    if (RAND_bytes(salt, SALT_SIZE) != 1 || RAND_bytes(iv, IV_SIZE) != 1) {
        goto cleanup;
    }
    
    /* Derive encryption key */
    if (!derive_key(passphrase, salt, SALT_SIZE, key, KEY_SIZE)) {
        goto cleanup;
    }
    
    /* Create cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }
    
    /* Initialize encryption */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        goto cleanup;
    }
    
    /* Allocate output buffer */
    total_size = HEADER_SIZE + SALT_SIZE + IV_SIZE + plaintext_len + TAG_SIZE;
    result = (unsigned char*)malloc(total_size);
    if (result == NULL) {
        goto cleanup;
    }
    
    /* Write header */
    memcpy(result, KEY_MAGIC, 4);
    {
        uint32_t version = KEY_VERSION;
        memcpy(result + 4, &version, 4);
    }
    
    /* Write salt and IV */
    memcpy(result + HEADER_SIZE, salt, SALT_SIZE);
    memcpy(result + HEADER_SIZE + SALT_SIZE, iv, IV_SIZE);
    
    /* Encrypt data */
    if (EVP_EncryptUpdate(ctx, result + HEADER_SIZE + SALT_SIZE + IV_SIZE,
                         &len, plaintext, (int)plaintext_len) != 1) {
        goto cleanup;
    }
    ciphertext_len = len;
    
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, result + HEADER_SIZE + SALT_SIZE + IV_SIZE + len,
                           &len) != 1) {
        goto cleanup;
    }
    ciphertext_len += len;
    
    /* Get authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE,
                           result + HEADER_SIZE + SALT_SIZE + IV_SIZE + ciphertext_len) != 1) {
        goto cleanup;
    }
    
    *output = result;
    *output_len = HEADER_SIZE + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE;
    success = 1;
    result = NULL; /* Prevent cleanup from freeing */
    
cleanup:
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    secure_clear(key, KEY_SIZE);
    secure_clear(salt, SALT_SIZE);
    secure_clear(iv, IV_SIZE);
    if (result != NULL) {
        secure_clear(result, total_size);
        free(result);
    }
    
    return success;
}

/* Decrypt key data using AES-256-GCM */
int decrypt_key_data(const char* passphrase, const unsigned char* encrypted,
                     size_t encrypted_len, unsigned char** plaintext, size_t* plaintext_len) {
    EVP_CIPHER_CTX* ctx = NULL;
    unsigned char key[KEY_SIZE];
    const unsigned char* salt;
    const unsigned char* iv;
    const unsigned char* ciphertext;
    const unsigned char* tag;
    size_t ciphertext_len;
    unsigned char* result = NULL;
    int len = 0;
    int total_len = 0;
    uint32_t version;
    int success = 0;
    
    /* Validate inputs */
    if (passphrase == NULL || encrypted == NULL || plaintext == NULL || 
        plaintext_len == NULL || encrypted_len < HEADER_SIZE + SALT_SIZE + IV_SIZE + TAG_SIZE) {
        return 0;
    }
    
    /* Initialize output */
    *plaintext = NULL;
    *plaintext_len = 0;
    
    /* Validate magic and version */
    if (memcmp(encrypted, KEY_MAGIC, 4) != 0) {
        return 0;
    }
    
    memcpy(&version, encrypted + 4, 4);
    if (version != KEY_VERSION) {
        return 0;
    }
    
    /* Extract components */
    salt = encrypted + HEADER_SIZE;
    iv = salt + SALT_SIZE;
    ciphertext = iv + IV_SIZE;
    ciphertext_len = encrypted_len - HEADER_SIZE - SALT_SIZE - IV_SIZE - TAG_SIZE;
    
    if (ciphertext_len > MAX_KEY_DATA_SIZE) {
        return 0;
    }
    
    tag = ciphertext + ciphertext_len;
    
    /* Derive decryption key */
    if (!derive_key(passphrase, salt, SALT_SIZE, key, KEY_SIZE)) {
        goto cleanup;
    }
    
    /* Create cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }
    
    /* Initialize decryption */
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        goto cleanup;
    }
    
    /* Allocate output buffer */
    result = (unsigned char*)malloc(ciphertext_len);
    if (result == NULL) {
        goto cleanup;
    }
    
    /* Decrypt data */
    if (EVP_DecryptUpdate(ctx, result, &len, ciphertext, (int)ciphertext_len) != 1) {
        goto cleanup;
    }
    total_len = len;
    
    /* Set expected tag for verification */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE,
                           (void*)tag) != 1) {
        goto cleanup;
    }
    
    /* Finalize decryption and verify tag */
    if (EVP_DecryptFinal_ex(ctx, result + len, &len) != 1) {
        /* Tag verification failed - clear and fail */
        goto cleanup;
    }
    total_len += len;
    
    *plaintext = result;
    *plaintext_len = total_len;
    success = 1;
    result = NULL; /* Prevent cleanup from freeing */
    
cleanup:
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    secure_clear(key, KEY_SIZE);
    if (result != NULL) {
        secure_clear(result, ciphertext_len);
        free(result);
    }
    
    return success;
}

/* Store encrypted key to file with atomic write */
int store_key(const char* filename, const char* passphrase, const char* key_data) {
    unsigned char* encrypted = NULL;
    size_t encrypted_len = 0;
    FILE* fp = NULL;
    char temp_filename[512];
    int success = 0;
    size_t key_data_len;
    
    /* Validate inputs */
    if (filename == NULL || passphrase == NULL || key_data == NULL ||
        strlen(filename) == 0 || strlen(passphrase) == 0 || strlen(key_data) == 0) {
        return 0;
    }
    
    /* Validate filename - no path traversal */
    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL ||
        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    key_data_len = strlen(key_data);\n    \n    /* Encrypt key data */\n    if (!encrypt_key_data(passphrase, (const unsigned char*)key_data,\n                         key_data_len, &encrypted, &encrypted_len)) {\n        return 0;\n    }\n    \n    /* Create temporary filename */\n    if (snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename) >= (int)sizeof(temp_filename)) {\n        goto cleanup;\n    }\n    \n    /* Write to temporary file */\n    fp = fopen(temp_filename, "wb");\n    if (fp == NULL) {\n        goto cleanup;\n    }\n    \n    if (fwrite(encrypted, 1, encrypted_len, fp) != encrypted_len) {\n        goto cleanup;\n    }\n    \n    if (fflush(fp) != 0) {\n        goto cleanup;\n    }\n    \n    if (fclose(fp) != 0) {\n        fp = NULL;\n        goto cleanup;\n    }\n    fp = NULL;\n    \n    /* Atomic rename */\n    if (rename(temp_filename, filename) != 0) {\n        remove(temp_filename);\n        goto cleanup;\n    }\n    \n    success = 1;\n    \ncleanup:\n    if (fp != NULL) {\n        fclose(fp);\n        remove(temp_filename);\n    }\n    if (encrypted != NULL) {\n        secure_clear(encrypted, encrypted_len);\n        free(encrypted);\n    }\n    \n    return success;\n}\n\n/* Load and decrypt key from file */\nint load_key(const char* filename, const char* passphrase, char** key_data) {\n    FILE* fp = NULL;\n    unsigned char* encrypted = NULL;\n    unsigned char* plaintext = NULL;\n    size_t plaintext_len = 0;\n    long file_size;\n    size_t read_size;\n    char* result = NULL;\n    int success = 0;\n    \n    /* Validate inputs */\n    if (filename == NULL || passphrase == NULL || key_data == NULL) {\n        return 0;\n    }\n    \n    /* Initialize output */\n    *key_data = NULL;\n    \n    /* Validate filename */\n    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL ||\n        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    /* Open file */\n    fp = fopen(filename, "rb");\n    if (fp == NULL) {\n        return 0;\n    }\n    \n    /* Get file size */\n    if (fseek(fp, 0, SEEK_END) != 0) {\n        goto cleanup;\n    }\n    \n    file_size = ftell(fp);\n    if (file_size <= 0 || file_size > (long)(MAX_KEY_DATA_SIZE + HEADER_SIZE + SALT_SIZE + IV_SIZE + TAG_SIZE)) {\n        goto cleanup;\n    }\n    \n    if (fseek(fp, 0, SEEK_SET) != 0) {\n        goto cleanup;\n    }\n    \n    /* Allocate buffer */\n    encrypted = (unsigned char*)malloc((size_t)file_size);\n    if (encrypted == NULL) {\n        goto cleanup;\n    }\n    \n    /* Read file */\n    read_size = fread(encrypted, 1, (size_t)file_size, fp);\n    if (read_size != (size_t)file_size) {\n        goto cleanup;\n    }\n    \n    /* Decrypt */\n    if (!decrypt_key_data(passphrase, encrypted, read_size, &plaintext, &plaintext_len)) {\n        goto cleanup;\n    }\n    \n    /* Allocate result string */\n    result = (char*)malloc(plaintext_len + 1);\n    if (result == NULL) {\n        goto cleanup;\n    }\n    \n    memcpy(result, plaintext, plaintext_len);\n    result[plaintext_len] = '\\0';\n    \n    *key_data = result;\n    success = 1;\n    result = NULL; /* Prevent cleanup from freeing */\n    \ncleanup:\n    if (fp != NULL) {\n        fclose(fp);\n    }\n    if (encrypted != NULL) {\n        secure_clear(encrypted, (size_t)file_size);\n        free(encrypted);\n    }\n    if (plaintext != NULL) {\n        secure_clear(plaintext, plaintext_len);\n        free(plaintext);\n    }\n    if (result != NULL) {\n        secure_clear(result, plaintext_len);\n        free(result);\n    }\n    \n    return success;\n}\n\nint main(void) {\n    char* loaded = NULL;\n    \n    /* Test 1: Store and load simple key */\n    printf("Test 1: Store and load simple key... ");\n    if (store_key("test_key1.enc", "SecurePass123!", "MySecretKey123")) {\n        if (load_key("test_key1.enc", "SecurePass123!", &loaded)) {\n            if (strcmp(loaded, "MySecretKey123") == 0) {\n                printf("PASSED\\n");\n            } else {\n                printf("FAILED\\n");\n            }\n            secure_clear(loaded, strlen(loaded));\n            free(loaded);\n            loaded = NULL;\n        } else {\n            printf("FAILED\\n");\n        }\n        remove("test_key1.enc");\n    } else {\n        printf("FAILED\\n");\n    }\n    \n    /* Test 2: Wrong passphrase should fail */\n    printf("Test 2: Wrong passphrase fails... ");\n    store_key("test_key2.enc", "CorrectPass", "SensitiveData");\n    if (!load_key("test_key2.enc", "WrongPass", &loaded)) {\n        printf("PASSED\\n");\n    } else {\n        printf("FAILED\\n");\n        if (loaded != NULL) {\n            secure_clear(loaded, strlen(loaded));\n            free(loaded);\n            loaded = NULL;\n        }\n    }\n    remove("test_key2.enc");\n    \n    /* Test 3: Store and load longer key data */\n    printf("Test 3: Store and load longer key... ");\n    {\n        char long_key[513];\n        memset(long_key, 'A', 512);\n        long_key[512] = '\\0';
        
        if (store_key("test_key3.enc", "LongDataPass", long_key)) {
            if (load_key("test_key3.enc", "LongDataPass", &loaded)) {
                if (strcmp(loaded, long_key) == 0) {
                    printf("PASSED\\n");
                } else {
                    printf("FAILED\\n");
                }
                secure_clear(loaded, strlen(loaded));
                free(loaded);
                loaded = NULL;
            } else {
                printf("FAILED\\n");
            }
            remove("test_key3.enc");
        } else {
            printf("FAILED\\n");
        }
    }
    
    /* Test 4: Invalid filename with path traversal */
    printf("Test 4: Path traversal rejected... ");
    if (!store_key("../test_key4.enc", "Pass", "Data")) {
        printf("PASSED\\n");
    } else {
        printf("FAILED\\n");
    }
    
    /* Test 5: Empty inputs should fail */
    printf("Test 5: Empty inputs rejected... ");
    if (!store_key("", "pass", "data") &&
        !store_key("file", "", "data") &&
        !store_key("file", "pass", "")) {
        printf("PASSED\\n");
    } else {
        printf("FAILED\\n");
    }
    
    return 0;
}
