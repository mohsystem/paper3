
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Magic number and version for file format validation
#define MAGIC "ENC1"
#define VERSION 1
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define PBKDF2_ITERATIONS 210000

// Struct to hold encrypted data
typedef struct {
    uint8_t* data;
    size_t length;
} EncryptedData;

// Function to securely clear memory
void secure_clear(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

// Derive key from passphrase using PBKDF2-HMAC-SHA-256
int derive_key(const char* passphrase, const uint8_t* salt, uint8_t* key) {
    if (passphrase == NULL || salt == NULL || key == NULL) {
        return 0;
    }
    
    size_t passphrase_len = strlen(passphrase);
    if (passphrase_len == 0 || passphrase_len > 1024) {
        return 0;
    }
    
    // Use PBKDF2 with HMAC-SHA-256
    int result = PKCS5_PBKDF2_HMAC(
        passphrase, (int)passphrase_len,
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE, key
    );
    
    return result;
}

// Encrypt message using AES-256-GCM
EncryptedData* encrypt_message(const char* message, const char* passphrase) {
    if (message == NULL || passphrase == NULL) {
        return NULL;
    }
    
    size_t message_len = strlen(message);
    if (message_len == 0 || message_len > 1000000) {  // Reasonable limit
        return NULL;
    }
    
    EVP_CIPHER_CTX* ctx = NULL;
    EncryptedData* result = NULL;
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t key[KEY_SIZE];
    uint8_t tag[TAG_SIZE];
    uint8_t* ciphertext = NULL;
    int len = 0;
    int ciphertext_len = 0;
    
    // Initialize all pointers
    memset(salt, 0, SALT_SIZE);
    memset(iv, 0, IV_SIZE);
    memset(key, 0, KEY_SIZE);
    memset(tag, 0, TAG_SIZE);
    
    // Generate cryptographically secure random salt and IV
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        goto cleanup;
    }
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        goto cleanup;
    }
    
    // Derive key from passphrase
    if (derive_key(passphrase, salt, key) != 1) {
        goto cleanup;
    }
    
    // Create and initialize cipher context
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }
    
    // Initialize encryption with AES-256-GCM
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        goto cleanup;
    }
    
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) {
        goto cleanup;
    }
    
    // Initialize key and IV
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        goto cleanup;
    }
    
    // Allocate buffer for ciphertext (same size as plaintext is sufficient for GCM)
    ciphertext = (uint8_t*)malloc(message_len);
    if (ciphertext == NULL) {
        goto cleanup;
    }
    
    // Encrypt the message
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (const uint8_t*)message, (int)message_len) != 1) {
        goto cleanup;
    }
    ciphertext_len = len;
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        goto cleanup;
    }
    ciphertext_len += len;
    
    // Get authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }
    
    // Build output format: [MAGIC][VERSION][SALT][IV][CIPHERTEXT][TAG]
    size_t total_size = 4 + 1 + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE;
    result = (EncryptedData*)malloc(sizeof(EncryptedData));
    if (result == NULL) {
        goto cleanup;
    }
    
    result->data = (uint8_t*)malloc(total_size);
    if (result->data == NULL) {
        free(result);
        result = NULL;
        goto cleanup;
    }
    
    result->length = total_size;
    
    // Assemble the encrypted data structure
    size_t offset = 0;
    memcpy(result->data + offset, MAGIC, 4);
    offset += 4;
    result->data[offset] = VERSION;
    offset += 1;
    memcpy(result->data + offset, salt, SALT_SIZE);
    offset += SALT_SIZE;
    memcpy(result->data + offset, iv, IV_SIZE);
    offset += IV_SIZE;
    memcpy(result->data + offset, ciphertext, ciphertext_len);
    offset += ciphertext_len;
    memcpy(result->data + offset, tag, TAG_SIZE);
    
cleanup:
    // Securely clear sensitive data
    secure_clear(key, KEY_SIZE);
    secure_clear(salt, SALT_SIZE);
    secure_clear(iv, IV_SIZE);
    secure_clear(tag, TAG_SIZE);
    
    if (ciphertext != NULL) {
        secure_clear(ciphertext, message_len);
        free(ciphertext);
    }
    
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    
    return result;
}

// Free encrypted data
void free_encrypted_data(EncryptedData* data) {
    if (data != NULL) {
        if (data->data != NULL) {
            secure_clear(data->data, data->length);
            free(data->data);
        }
        free(data);
    }
}

int main() {
    // Test cases
    const char* test_messages[] = {
        "Hello, World!",
        "This is a secret message.",
        "AES-256-GCM encryption test",
        "1234567890",
        "Special chars: !@#$%^&*()"
    };
    
    const char* test_passphrase = "MySecurePassphrase123!";
    
    printf("AES-256-GCM Encryption Test\\n");
    printf("===========================\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        printf("Message: %s\\n", test_messages[i]);
        
        EncryptedData* encrypted = encrypt_message(test_messages[i], test_passphrase);
        
        if (encrypted != NULL) {
            printf("Encrypted successfully! Size: %zu bytes\\n", encrypted->length);
            printf("Format: [MAGIC(4)][VERSION(1)][SALT(16)][IV(12)][CIPHERTEXT][TAG(16)]\\n");
            
            // Display first 32 bytes in hex for verification
            printf("First 32 bytes (hex): ");
            for (size_t j = 0; j < 32 && j < encrypted->length; j++) {
                printf("%02x", encrypted->data[j]);
            }
            printf("\\n");
            
            free_encrypted_data(encrypted);
        } else {
            printf("Encryption failed!\\n");
        }
        
        printf("\\n");
    }
    
    return 0;
}
