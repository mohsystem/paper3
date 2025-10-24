
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <stdlib.h>

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* buffer, size_t length) {
    size_t output_length = 4 * ((length + 2) / 3);
    char* encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? buffer[i++] : 0;
        uint32_t octet_b = i < length ? buffer[i++] : 0;
        uint32_t octet_c = i < length ? buffer[i++] : 0;
        
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    int padding = (3 - length % 3) % 3;
    for (i = 0; i < padding; i++)
        encoded_data[output_length - 1 - i] = '=';
    
    encoded_data[output_length] = '\\0';
    return encoded_data;
}

char* encryptAES(const char* key, const char* plainText) {
    unsigned char keyBytes[16];
    memset(keyBytes, 0, 16);
    size_t keyLen = strlen(key);
    memcpy(keyBytes, key, keyLen < 16 ? keyLen : 16);
    
    unsigned char iv[16];
    RAND_bytes(iv, 16);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, keyBytes, iv);
    
    int len;
    int ciphertext_len;
    unsigned char ciphertext[1024];
    
    EVP_EncryptUpdate(ctx, ciphertext, &len, 
                      (unsigned char*)plainText, strlen(plainText));
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    unsigned char* combined = malloc(16 + ciphertext_len);
    memcpy(combined, iv, 16);
    memcpy(combined + 16, ciphertext, ciphertext_len);
    
    char* encoded = base64_encode(combined, 16 + ciphertext_len);
    free(combined);
    
    return encoded;
}

int main() {
    const char* keys[] = {
        "mySecretKey12345",
        "anotherKey456789",
        "testKey123456789",
        "secureKey9876543",
        "randomKey1122334"
    };
    
    const char* plainTexts[] = {
        "Hello World",
        "This is a test message",
        "AES encryption in CBC mode",
        "Secure data transmission",
        "Testing encryption"
    };
    
    for (int i = 0; i < 5; i++) {
        char* encrypted = encryptAES(keys[i], plainTexts[i]);
        printf("Test Case %d:\\n", i + 1);
        printf("Key: %s\\n", keys[i]);
        printf("Plain Text: %s\\n", plainTexts[i]);
        printf("Encrypted: %s\\n", encrypted);
        printf("\\n");
        free(encrypted);
    }
    
    return 0;
}
