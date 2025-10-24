
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

class Task72 {
private:
    // SECURITY WARNING: Static IV is a critical vulnerability in production!
    // This should NEVER be used in real applications.
    static const unsigned char STATIC_IV[AES_BLOCK_SIZE];
    
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
    
    static std::vector<unsigned char> pkcs7Pad(const std::vector<unsigned char>& data, size_t blockSize) {
        size_t paddingLength = blockSize - (data.size() % blockSize);
        std::vector<unsigned char> padded = data;
        padded.insert(padded.end(), paddingLength, static_cast<unsigned char>(paddingLength));
        return padded;
    }
    
public:
    /**
     * Encrypts data using AES-CBC with PKCS7 padding
     * WARNING: Uses static IV - NOT SECURE for production use!
     */
    static std::string encryptData(const std::string& data, const std::vector<unsigned char>& key) {
        if (data.empty()) {
            throw std::invalid_argument("Data cannot be empty");
        }
        
        if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
            throw std::invalid_argument("Key must be 16, 24, or 32 bytes for AES");
        }
        
        // Step 1: Use static IV
        // Step 2: Initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }
        
        const EVP_CIPHER* cipher;
        if (key.size() == 16) {
            cipher = EVP_aes_128_cbc();
        } else if (key.size() == 24) {
            cipher = EVP_aes_192_cbc();
        } else {
            cipher = EVP_aes_256_cbc();
        }
        
        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key.data(), STATIC_IV) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        // Step 3 & 4: Padding and encryption (handled by OpenSSL)
        std::vector<unsigned char> plaintext(data.begin(), data.end());
        std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
        
        int len = 0;
        int ciphertext_len = 0;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        // Step 5: Return encrypted data as Base64
        return base64Encode(ciphertext.data(), ciphertext_len);
    }
    
    static std::vector<unsigned char> generateSecureKey(int keySize = 256) {
        std::vector<unsigned char> key(keySize / 8);
        if (RAND_bytes(key.data(), key.size()) != 1) {
            throw std::runtime_error("Failed to generate secure key");
        }
        return key;
    }
};

// Initialize static IV (all zeros - INSECURE!)
const unsigned char Task72::STATIC_IV[AES_BLOCK_SIZE] = {0};

int main() {
    std::cout << "=== AES-CBC Encryption Demo ===" << std::endl;
    std::cout << "WARNING: This uses a static IV - NOT SECURE!\\n" << std::endl;
    
    try {
        // Generate a secure key for testing
        std::vector<unsigned char> key = Task72::generateSecureKey(256);
        
        // Test Case 1: Simple text
        std::string test1 = "Hello, World!";
        std::string encrypted1 = Task72::encryptData(test1, key);
        std::cout << "Test 1 - Input: " << test1 << std::endl;
        std::cout << "Encrypted: " << encrypted1 << "\\n" << std::endl;
        
        // Test Case 2: Longer text
        std::string test2 = "This is a longer message that will span multiple blocks.";
        std::string encrypted2 = Task72::encryptData(test2, key);
        std::cout << "Test 2 - Input: " << test2 << std::endl;
        std::cout << "Encrypted: " << encrypted2 << "\\n" << std::endl;
        
        // Test Case 3: Special characters
        std::string test3 = "Special chars: !@#$%^&*()_+-={}[]|:;<>?,./";
        std::string encrypted3 = Task72::encryptData(test3, key);
        std::cout << "Test 3 - Input: " << test3 << std::endl;
        std::cout << "Encrypted: " << encrypted3 << "\\n" << std::endl;
        
        // Test Case 4: Unicode text
        std::string test4 = "Unicode: Hello World";
        std::string encrypted4 = Task72::encryptData(test4, key);
        std::cout << "Test 4 - Input: " << test4 << std::endl;
        std::cout << "Encrypted: " << encrypted4 << "\\n" << std::endl;
        
        // Test Case 5: Demonstrating static IV vulnerability
        std::string test5a = "Same message";
        std::string test5b = "Same message";
        std::string encrypted5a = Task72::encryptData(test5a, key);
        std::string encrypted5b = Task72::encryptData(test5b, key);
        std::cout << "Test 5 - Demonstrating Static IV Vulnerability:" << std::endl;
        std::cout << "Message A: " << test5a << std::endl;
        std::cout << "Encrypted A: " << encrypted5a << std::endl;
        std::cout << "Message B: " << test5b << std::endl;
        std::cout << "Encrypted B: " << encrypted5b << std::endl;
        std::cout << "Same ciphertext? " << (encrypted5a == encrypted5b ? "YES" : "NO") << std::endl;
        std::cout << "^ This is a CRITICAL security flaw!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Encryption error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
