
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdlib.h>

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

char* encryptMessage(const char* message, const char* secretKey) {
    unsigned char key[16] = {0};
    size_t keyLen = strlen(secretKey);
    memcpy(key, secretKey, keyLen < 16 ? keyLen : 16);
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
    
    int messageLen = strlen(message);
    int ciphertext_len = messageLen + AES_BLOCK_SIZE;
    unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_len);
    int len;
    
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message, messageLen);
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    
    char* result = base64_encode(ciphertext, ciphertext_len);
    
    free(ciphertext);
    EVP_CIPHER_CTX_free(ctx);
    
    return result;
}

int main() {
    char* encrypted;
    
    // Test case 1
    printf("Test 1:\\n");
    printf("Message: Hello World\\n");
    printf("Key: mySecretKey12345\\n");
    encrypted = encryptMessage("Hello World", "mySecretKey12345");
    printf("Encrypted: %s\\n\\n", encrypted);
    free(encrypted);
    
    // Test case 2
    printf("Test 2:\\n");
    printf("Message: This is a secret message\\n");
    printf("Key: password123\\n");
    encrypted = encryptMessage("This is a secret message", "password123");
    printf("Encrypted: %s\\n\\n", encrypted);
    free(encrypted);
    
    // Test case 3
    printf("Test 3:\\n");
    printf("Message: OpenAI GPT\\n");
    printf("Key: key123\\n");
    encrypted = encryptMessage("OpenAI GPT", "key123");
    printf("Encrypted: %s\\n\\n", encrypted);
    free(encrypted);
    
    // Test case 4
    printf("Test 4:\\n");
    printf("Message: Encryption test 2024\\n");
    printf("Key: supersecret\\n");
    encrypted = encryptMessage("Encryption test 2024", "supersecret");
    printf("Encrypted: %s\\n\\n", encrypted);
    free(encrypted);
    
    // Test case 5
    printf("Test 5:\\n");
    printf("Message: 12345\\n");
    printf("Key: numerickey\\n");
    encrypted = encryptMessage("12345", "numerickey");
    printf("Encrypted: %s\\n", encrypted);
    free(encrypted);
    
    return 0;
}
