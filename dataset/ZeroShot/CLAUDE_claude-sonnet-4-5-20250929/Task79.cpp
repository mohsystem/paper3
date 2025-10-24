
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

std::string base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

std::string encrypt(const std::string& message, const std::string& secretKey) {
    // Prepare 16-byte key
    unsigned char key[16] = {0};
    memcpy(key, secretKey.c_str(), std::min(secretKey.length(), (size_t)16));
    
    // Create cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
    
    // Allocate buffer for encrypted data
    int len;
    int ciphertext_len;
    unsigned char ciphertext[1024];
    
    // Encrypt the message
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message.c_str(), message.length());
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Return Base64 encoded string
    return base64_encode(ciphertext, ciphertext_len);
}

int main() {
    // Test case 1
    std::string result1 = encrypt("Hello World", "mySecretKey12345");
    std::cout << "Test 1 - Encrypted: " << result1 << std::endl;
    
    // Test case 2
    std::string result2 = encrypt("Secure Message", "password1234567");
    std::cout << "Test 2 - Encrypted: " << result2 << std::endl;
    
    // Test case 3
    std::string result3 = encrypt("Testing 123", "key123456789012");
    std::cout << "Test 3 - Encrypted: " << result3 << std::endl;
    
    // Test case 4
    std::string result4 = encrypt("Encryption Test", "secretKey098765");
    std::cout << "Test 4 - Encrypted: " << result4 << std::endl;
    
    // Test case 5
    std::string result5 = encrypt("Java Crypto", "myKey1234567890");
    std::cout << "Test 5 - Encrypted: " << result5 << std::endl;
    
    return 0;
}
