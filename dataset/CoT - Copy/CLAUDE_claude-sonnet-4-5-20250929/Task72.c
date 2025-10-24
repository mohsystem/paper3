
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

/* SECURITY WARNING: Static IV is a critical vulnerability in production!
 * This should NEVER be used in real applications.
 */
static const unsigned char STATIC_IV[AES_BLOCK_SIZE] = {0};

/**
 * Base64 encode function
 */
char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    char* result = (char*)malloc(bufferPtr->length + 1);
    if (result) {
        memcpy(result, bufferPtr->data, bufferPtr->length);
        result[bufferPtr->length] = '\\0';
    }
    
    BIO_free_all(bio);
    return result;
}

/**
 * Encrypts data using AES-CBC with PKCS7 padding
 * WARNING: Uses static IV - NOT SECURE for production use!
 * 
 * Returns: Base64 encoded encrypted data (caller must free)
 */
char* encrypt_data(const char* data, const unsigned char* key, size_t key_len) {
    if (!data || !key) {
        fprintf(stderr, "Error: Invalid input parameters\\n");
        return NULL;
    }
    
    if (key_len != 16 && key_len != 24 && key_len != 32) {
        fprintf(stderr, "Error: Key must be 16, 24, or 32 bytes for AES\\n");
        return NULL;
    }
    
    size_t data_len = strlen(data);
    if (data_len == 0) {
        fprintf(stderr, "Error: Data cannot be empty\\n");
        return NULL;
    }
    
    /* Step 1: Use static IV */
    /* Step 2: Initialize cipher context */
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create cipher context\\n");
        return NULL;
    }
    
    const EVP_CIPHER* cipher;
    if (key_len == 16) {
        cipher = EVP_aes_128_cbc();
    } else if (key_len == 24) {
        cipher = EVP_aes_192_cbc();
    } else {
        cipher = EVP_aes_256_cbc();
    }
    
    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, STATIC_IV) != 1) {
        fprintf(stderr, "Error: Failed to initialize encryption\\n");
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    /* Step 3 & 4: Padding and encryption (handled by OpenSSL) */
    unsigned char* ciphertext = (unsigned char*)malloc(data_len + AES_BLOCK_SIZE);
    if (!ciphertext) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    int len = 0;
    int ciphertext_len = 0;
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)data, data_len) != 1) {
        fprintf(stderr, "Error: Encryption failed\\n");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Error: Encryption finalization failed\\n");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    /* Step 5: Return encrypted data as Base64 */
    char* result = base64_encode(ciphertext, ciphertext_len);
    free(ciphertext);
    
    return result;
}

/**
 * Generates a secure random key for AES encryption
 */
int generate_secure_key(unsigned char* key, int key_size_bits) {
    int key_size_bytes = key_size_bits / 8;
    if (RAND_bytes(key, key_size_bytes) != 1) {
        fprintf(stderr, "Error: Failed to generate secure key\\n");
        return 0;
    }
    return 1;
}

int main() {
    printf("=== AES-CBC Encryption Demo ===\\n");
    printf("WARNING: This uses a static IV - NOT SECURE!\\n\\n");
    
    /* Generate a secure key for testing */
    unsigned char key[32];
    if (!generate_secure_key(key, 256)) {
        return 1;
    }
    
    /* Test Case 1: Simple text */
    const char* test1 = "Hello, World!";
    char* encrypted1 = encrypt_data(test1, key, 32);
    if (encrypted1) {
        printf("Test 1 - Input: %s\\n", test1);
        printf("Encrypted: %s\\n\\n", encrypted1);
        free(encrypted1);
    }
    
    /* Test Case 2: Longer text */
    const char* test2 = "This is a longer message that will span multiple blocks.";
    char* encrypted2 = encrypt_data(test2, key, 32);
    if (encrypted2) {
        printf("Test 2 - Input: %s\\n", test2);
        printf("Encrypted: %s\\n\\n", encrypted2);
        free(encrypted2);
    }
    
    /* Test Case 3: Special characters */
    const char* test3 = "Special chars: !@#$%^&*()_+-={}[]|:;<>?,./";
    char* encrypted3 = encrypt_data(test3, key, 32);
    if (encrypted3) {
        printf("Test 3 - Input: %s\\n", test3);
        printf("Encrypted: %s\\n\\n", encrypted3);
        free(encrypted3);
    }
    
    /* Test Case 4: Another message */
    const char* test4 = "Another test message for encryption";
    char* encrypted4 = encrypt_data(test4, key, 32);
    if (encrypted4) {
        printf("Test 4 - Input: %s\\n", test4);
        printf("Encrypted: %s\\n\\n", encrypted4);
        free(encrypted4);
    }
    
    /* Test Case 5: Demonstrating static IV vulnerability */
    const char* test5a = "Same message";
    const char* test5b = "Same message";
    char* encrypted5a = encrypt_data(test5a, key, 32);
    char* encrypted5b = encrypt_data(test5b, key, 32);
    
    if (encrypted5a && encrypted5b) {
        printf("Test 5 - Demonstrating Static IV Vulnerability:\\n");
        printf("Message A: %s\\n", test5a);
        printf("Encrypted A: %s\\n", encrypted5a);
        printf("Message B: %s\\n", test5b);
        printf("Encrypted B: %s\\n", encrypted5b);
        printf("Same ciphertext? %s\\n", 
               strcmp(encrypted5a, encrypted5b) == 0 ? "YES" : "NO");
        printf("^ This is a CRITICAL security flaw!\\n");
        
        free(encrypted5a);
        free(encrypted5b);
    }
    
    return 0;
}
