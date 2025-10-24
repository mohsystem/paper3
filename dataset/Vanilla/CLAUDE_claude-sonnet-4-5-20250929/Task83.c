
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* buffer, size_t length) {
    size_t output_length = 4 * ((length + 2) / 3);
    char* encoded = (char*)malloc(output_length + 1);
    if (!encoded) return NULL;
    
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t pos = 0;

    while (length--) {
        char_array_3[i++] = *(buffer++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                encoded[pos++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            encoded[pos++] = base64_chars[char_array_4[j]];

        while(i++ < 3)
            encoded[pos++] = '=';
    }

    encoded[pos] = '\\0';
    return encoded;
}

char* encryptAES(const char* plainText, const char* key) {
    unsigned char keyBytes[16] = {0};
    size_t keyLen = strlen(key);
    memcpy(keyBytes, key, keyLen < 16 ? keyLen : 16);
    
    unsigned char iv[16];
    RAND_bytes(iv, 16);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, keyBytes, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    int plainLen = strlen(plainText);
    unsigned char* encrypted = (unsigned char*)malloc(plainLen + AES_BLOCK_SIZE);
    int len;
    int ciphertext_len;
    
    if (EVP_EncryptUpdate(ctx, encrypted, &len, (unsigned char*)plainText, plainLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(encrypted);
        return NULL;
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, encrypted + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(encrypted);
        return NULL;
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    unsigned char* combined = (unsigned char*)malloc(16 + ciphertext_len);
    memcpy(combined, iv, 16);
    memcpy(combined + 16, encrypted, ciphertext_len);
    
    char* result = base64_encode(combined, 16 + ciphertext_len);
    
    free(encrypted);
    free(combined);
    
    return result;
}

int main() {
    // Test case 1
    char* encrypted1 = encryptAES("Hello World", "mySecretKey12345");
    printf("Test 1 - Encrypted: %s\\n", encrypted1);
    free(encrypted1);
    
    // Test case 2
    char* encrypted2 = encryptAES("Sensitive Data", "password1234567");
    printf("Test 2 - Encrypted: %s\\n", encrypted2);
    free(encrypted2);
    
    // Test case 3
    char* encrypted3 = encryptAES("AES Encryption", "key123456789012");
    printf("Test 3 - Encrypted: %s\\n", encrypted3);
    free(encrypted3);
    
    // Test case 4
    char* encrypted4 = encryptAES("Test Message", "secretKey123456");
    printf("Test 4 - Encrypted: %s\\n", encrypted4);
    free(encrypted4);
    
    // Test case 5
    char* encrypted5 = encryptAES("Cipher Block", "encryptionKey16");
    printf("Test 5 - Encrypted: %s\\n", encrypted5);
    free(encrypted5);
    
    return 0;
}
