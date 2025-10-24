
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

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
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = '\\0';
    
    BIO_free_all(bio);
    
    return result;
}

char* encrypt(const char* message, const char* secretKey) {
    // Prepare 16-byte key
    unsigned char key[16] = {0};
    size_t keyLen = strlen(secretKey);
    memcpy(key, secretKey, keyLen < 16 ? keyLen : 16);
    
    // Create cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
    
    // Allocate buffer for encrypted data
    int len;
    int ciphertext_len;
    unsigned char ciphertext[1024];
    
    // Encrypt the message
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message, strlen(message));
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Return Base64 encoded string
    return base64_encode(ciphertext, ciphertext_len);
}

int main() {
    // Test case 1
    char* result1 = encrypt("Hello World", "mySecretKey12345");
    printf("Test 1 - Encrypted: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = encrypt("Secure Message", "password1234567");
    printf("Test 2 - Encrypted: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = encrypt("Testing 123", "key123456789012");
    printf("Test 3 - Encrypted: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = encrypt("Encryption Test", "secretKey098765");
    printf("Test 4 - Encrypted: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = encrypt("Java Crypto", "myKey1234567890");
    printf("Test 5 - Encrypted: %s\\n", result5);
    free(result5);
    
    return 0;
}
