
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

/* Note: This is a simplified example for educational purposes.
   Production code requires proper key management, IV usage,
   memory protection, and secure coding practices. */

typedef struct {
    unsigned char key[16];
} SecureStorage;

void init_secure_storage(SecureStorage* storage) {
    // In production, use proper key derivation
    memset(storage->key, 0x42, 16);
}

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < 32 && i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void encrypt_data(SecureStorage* storage, const char* plaintext, unsigned char* ciphertext, int* len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int tmp_len;
    
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, storage->key, NULL);
    EVP_EncryptUpdate(ctx, ciphertext, len, (unsigned char*)plaintext, strlen(plaintext));
    EVP_EncryptFinal_ex(ctx, ciphertext + *len, &tmp_len);
    *len += tmp_len;
    
    EVP_CIPHER_CTX_free(ctx);
}

int main() {
    SecureStorage storage;
    init_secure_storage(&storage);
    
    printf("Test 1: Initialize secure storage\\n");
    printf("Secure storage initialized\\n\\n");
    
    printf("Test 2: Hash password\\n");
    char hashed[65];
    hash_password("myPassword123", hashed);
    printf("Hashed: %.40s...\\n\\n", hashed);
    
    printf("Test 3: Encrypt data\\n");
    unsigned char encrypted[1024];
    int enc_len;
    encrypt_data(&storage, "SensitiveData", encrypted, &enc_len);
    printf("Data encrypted (%d bytes)\\n\\n", enc_len);
    
    printf("Test 4: Multiple operations\\n");
    hash_password("anotherPassword", hashed);
    printf("Another hash generated\\n\\n");
    
    printf("Test 5: Security notice\\n");
    printf("WARNING: Use proper security measures in production\\n");
    
    return 0;
}
