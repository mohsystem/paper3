
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define IV_SIZE 16
#define MAX_ENCRYPTED_SIZE 1024

typedef struct {
    unsigned char* encryptedData;
    int encryptedLen;
    unsigned char iv[IV_SIZE];
} EncryptionResult;

void freeEncryptionResult(EncryptionResult* result) {
    if (result && result->encryptedData) {
        free(result->encryptedData);
        result->encryptedData = NULL;
    }
}

char* base64Encode(const unsigned char* data, int len) {
    static const char base64Chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    int outputLen = 4 * ((len + 2) / 3);
    char* result = (char*)malloc(outputLen + 1);
    if (!result) return NULL;
    
    int i, j;
    for (i = 0, j = 0; i < len;) {
        unsigned int octet_a = i < len ? data[i++] : 0;
        unsigned int octet_b = i < len ? data[i++] : 0;
        unsigned int octet_c = i < len ? data[i++] : 0;
        
        unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        result[j++] = base64Chars[(triple >> 18) & 0x3F];
        result[j++] = base64Chars[(triple >> 12) & 0x3F];
        result[j++] = base64Chars[(triple >> 6) & 0x3F];
        result[j++] = base64Chars[triple & 0x3F];
    }
    
    int mod = len % 3;
    if (mod > 0) {
        for (i = 0; i < 3 - mod; i++) {
            result[outputLen - 1 - i] = '=';
        }
    }
    
    result[outputLen] = '\\0';
    return result;
}

int encryptData(const unsigned char* plaintext, int plaintextLen,
                const unsigned char* key, int keyLen,
                EncryptionResult* result) {
    if (!plaintext || plaintextLen <= 0) {
        fprintf(stderr, "Invalid plaintext\\n");
        return 0;
    }
    
    if (!key || (keyLen != 16 && keyLen != 24 && keyLen != 32)) {
        fprintf(stderr, "Key must be 16, 24, or 32 bytes\\n");
        return 0;
    }
    
    // Generate random IV
    if (RAND_bytes(result->iv, IV_SIZE) != 1) {
        fprintf(stderr, "Failed to generate random IV\\n");
        return 0;
    }
    
    // Create cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context\\n");
        return 0;
    }
    
    const EVP_CIPHER* cipher;
    if (keyLen == 16) {
        cipher = EVP_aes_128_cbc();
    } else if (keyLen == 24) {
        cipher = EVP_aes_192_cbc();
    } else {
        cipher = EVP_aes_256_cbc();
    }
    
    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, result->iv) != 1) {
        fprintf(stderr, "Failed to initialize encryption\\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    // Allocate buffer for encrypted data
    result->encryptedData = (unsigned char*)malloc(plaintextLen + EVP_CIPHER_block_size(cipher));
    if (!result->encryptedData) {
        fprintf(stderr, "Memory allocation failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    int len;
    if (EVP_EncryptUpdate(ctx, result->encryptedData, &len, plaintext, plaintextLen) != 1) {
        fprintf(stderr, "Encryption failed\\n");
        free(result->encryptedData);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    result->encryptedLen = len;
    
    if (EVP_EncryptFinal_ex(ctx, result->encryptedData + len, &len) != 1) {
        fprintf(stderr, "Encryption finalization failed\\n");
        free(result->encryptedData);
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    result->encryptedLen += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

int generateSecureKey(unsigned char* key, int keySize) {
    if (keySize != 128 && keySize != 192 && keySize != 256) {
        fprintf(stderr, "Key size must be 128, 192, or 256 bits\\n");
        return 0;
    }
    
    int keyBytes = keySize / 8;
    if (RAND_bytes(key, keyBytes) != 1) {
        fprintf(stderr, "Failed to generate random key\\n");
        return 0;
    }
    
    return keyBytes;
}

int main() {
    printf("AES CBC Encryption Test Cases\\n\\n");
    
    // Test Case 1: Standard encryption with 256-bit key
    printf("Test Case 1: 256-bit key encryption\\n");
    unsigned char key1[32];
    int keyLen1 = generateSecureKey(key1, 256);
    const char* plaintext1 = "Hello, Secure World!";
    EncryptionResult result1;
    
    if (encryptData((unsigned char*)plaintext1, strlen(plaintext1), key1, keyLen1, &result1)) {
        char* encrypted1 = base64Encode(result1.encryptedData, result1.encryptedLen);
        char* iv1 = base64Encode(result1.iv, IV_SIZE);
        printf("Plaintext: %s\\n", plaintext1);
        printf("Encrypted (Base64): %s\\n", encrypted1);
        printf("IV (Base64): %s\\n\\n", iv1);
        free(encrypted1);
        free(iv1);
        freeEncryptionResult(&result1);
    }
    
    // Test Case 2: Encryption with 128-bit key
    printf("Test Case 2: 128-bit key encryption\\n");
    unsigned char key2[16];
    int keyLen2 = generateSecureKey(key2, 128);
    const char* plaintext2 = "Sensitive Data 123";
    EncryptionResult result2;
    
    if (encryptData((unsigned char*)plaintext2, strlen(plaintext2), key2, keyLen2, &result2)) {
        char* encrypted2 = base64Encode(result2.encryptedData, result2.encryptedLen);
        char* iv2 = base64Encode(result2.iv, IV_SIZE);
        printf("Plaintext: %s\\n", plaintext2);
        printf("Encrypted (Base64): %s\\n", encrypted2);
        printf("IV (Base64): %s\\n\\n", iv2);
        free(encrypted2);
        free(iv2);
        freeEncryptionResult(&result2);
    }
    
    // Test Case 3: Large data encryption
    printf("Test Case 3: Large data encryption\\n");
    unsigned char key3[32];
    int keyLen3 = generateSecureKey(key3, 256);
    const char* plaintext3 = "This is a much longer message that contains multiple sentences. "
                            "It demonstrates the encryption of larger data blocks using AES CBC mode.";
    EncryptionResult result3;
    
    if (encryptData((unsigned char*)plaintext3, strlen(plaintext3), key3, keyLen3, &result3)) {
        char* encrypted3 = base64Encode(result3.encryptedData, result3.encryptedLen);
        char* iv3 = base64Encode(result3.iv, IV_SIZE);
        printf("Plaintext length: %lu bytes\\n", strlen(plaintext3));
        printf("Encrypted (Base64): %s\\n", encrypted3);
        printf("IV (Base64): %s\\n\\n", iv3);
        free(encrypted3);
        free(iv3);
        freeEncryptionResult(&result3);
    }
    
    // Test Case 4: Special characters encryption
    printf("Test Case 4: Special characters encryption\\n");
    unsigned char key4[24];
    int keyLen4 = generateSecureKey(key4, 192);
    const char* plaintext4 = "Special chars: @#$%^&*()_+-={}[]|:;<>?,./~`";
    EncryptionResult result4;
    
    if (encryptData((unsigned char*)plaintext4, strlen(plaintext4), key4, keyLen4, &result4)) {
        char* encrypted4 = base64Encode(result4.encryptedData, result4.encryptedLen);
        char* iv4 = base64Encode(result4.iv, IV_SIZE);
        printf("Plaintext: %s\\n", plaintext4);
        printf("Encrypted (Base64): %s\\n", encrypted4);
        printf("IV (Base64): %s\\n\\n", iv4);
        free(encrypted4);
        free(iv4);
        freeEncryptionResult(&result4);
    }
    
    // Test Case 5: Verify different IVs produce different ciphertexts
    printf("Test Case 5: Same plaintext, different IVs\\n");
    unsigned char key5[32];
    int keyLen5 = generateSecureKey(key5, 256);
    const char* plaintext5 = "Same message";
    EncryptionResult result5a, result5b;
    
    if (encryptData((unsigned char*)plaintext5, strlen(plaintext5), key5, keyLen5, &result5a) &&
        encryptData((unsigned char*)plaintext5, strlen(plaintext5), key5, keyLen5, &result5b)) {
        char* encrypted5a = base64Encode(result5a.encryptedData, result5a.encryptedLen);
        char* encrypted5b = base64Encode(result5b.encryptedData, result5b.encryptedLen);
        printf("Plaintext: %s\\n", plaintext5);
        printf("Encrypted 1 (Base64): %s\\n", encrypted5a);
        printf("Encrypted 2 (Base64): %s\\n", encrypted5b);
        printf("Are ciphertexts different? %s\\n",
               strcmp(encrypted5a, encrypted5b) != 0 ? "true" : "false");
        free(encrypted5a);
        free(encrypted5b);
        freeEncryptionResult(&result5a);
        freeEncryptionResult(&result5b);
    }
    
    return 0;
}
