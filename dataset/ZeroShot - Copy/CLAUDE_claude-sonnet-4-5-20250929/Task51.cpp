
#include <iostream>
#include <string>
#include <vector>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>
#include <iomanip>

class Task51 {
public:
    static std::string encrypt(const std::string& plainText, const std::string& secretKey) {
        std::vector<unsigned char> key = generateKey(secretKey);
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key.data(), NULL) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption initialization failed");
        }
        
        std::vector<unsigned char> ciphertext(plainText.size() + AES_BLOCK_SIZE);
        int len = 0, ciphertext_len = 0;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                             (unsigned char*)plainText.c_str(), plainText.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption update failed");
        }
        ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        return base64Encode(ciphertext.data(), ciphertext_len);
    }
    
    static std::string decrypt(const std::string& encryptedText, const std::string& secretKey) {
        std::vector<unsigned char> key = generateKey(secretKey);
        std::vector<unsigned char> ciphertext = base64Decode(encryptedText);
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        if (EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key.data(), NULL) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption initialization failed");
        }
        
        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0, plaintext_len = 0;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, 
                             ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption update failed");
        }
        plaintext_len = len;
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption finalization failed");
        }
        plaintext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        return std::string((char*)plaintext.data(), plaintext_len);
    }
    
private:
    static std::vector<unsigned char> generateKey(const std::string& secretKey) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)secretKey.c_str(), secretKey.length(), hash);
        return std::vector<unsigned char>(hash, hash + 16);
    }
    
    static std::string base64Encode(const unsigned char* buffer, size_t length) {
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
    
    static std::vector<unsigned char> base64Decode(const std::string& encoded) {
        BIO *bio, *b64;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new_mem_buf(encoded.c_str(), encoded.length());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        
        std::vector<unsigned char> result(encoded.length());
        int decodedLength = BIO_read(bio, result.data(), encoded.length());
        BIO_free_all(bio);
        
        result.resize(decodedLength);
        return result;
    }
};

int main() {
    // Test case 1
    std::string text1 = "Hello World";
    std::string key1 = "mySecretKey123";
    std::string encrypted1 = Task51::encrypt(text1, key1);
    std::string decrypted1 = Task51::decrypt(encrypted1, key1);
    std::cout << "Test 1:" << std::endl;
    std::cout << "Original: " << text1 << std::endl;
    std::cout << "Encrypted: " << encrypted1 << std::endl;
    std::cout << "Decrypted: " << decrypted1 << std::endl << std::endl;
    
    // Test case 2
    std::string text2 = "Secure Programming 2024";
    std::string key2 = "strongPassword!@#";
    std::string encrypted2 = Task51::encrypt(text2, key2);
    std::string decrypted2 = Task51::decrypt(encrypted2, key2);
    std::cout << "Test 2:" << std::endl;
    std::cout << "Original: " << text2 << std::endl;
    std::cout << "Encrypted: " << encrypted2 << std::endl;
    std::cout << "Decrypted: " << decrypted2 << std::endl << std::endl;
    
    // Test case 3
    std::string text3 = "12345 Special!@#$%";
    std::string key3 = "encryptionKey789";
    std::string encrypted3 = Task51::encrypt(text3, key3);
    std::string decrypted3 = Task51::decrypt(encrypted3, key3);
    std::cout << "Test 3:" << std::endl;
    std::cout << "Original: " << text3 << std::endl;
    std::cout << "Encrypted: " << encrypted3 << std::endl;
    std::cout << "Decrypted: " << decrypted3 << std::endl << std::endl;
    
    // Test case 4
    std::string text4 = "The quick brown fox jumps over the lazy dog";
    std::string key4 = "testKey2024";
    std::string encrypted4 = Task51::encrypt(text4, key4);
    std::string decrypted4 = Task51::decrypt(encrypted4, key4);
    std::cout << "Test 4:" << std::endl;
    std::cout << "Original: " << text4 << std::endl;
    std::cout << "Encrypted: " << encrypted4 << std::endl;
    std::cout << "Decrypted: " << decrypted4 << std::endl << std::endl;
    
    // Test case 5
    std::string text5 = "AES Encryption Test";
    std::string key5 = "anotherSecretKey";
    std::string encrypted5 = Task51::encrypt(text5, key5);
    std::string decrypted5 = Task51::decrypt(encrypted5, key5);
    std::cout << "Test 5:" << std::endl;
    std::cout << "Original: " << text5 << std::endl;
    std::cout << "Encrypted: " << encrypted5 << std::endl;
    std::cout << "Decrypted: " << decrypted5 << std::endl;
    
    return 0;
}
