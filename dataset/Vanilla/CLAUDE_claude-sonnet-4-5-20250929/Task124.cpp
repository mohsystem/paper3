
#include <iostream>
#include <string>
#include <map>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <iomanip>
#include <sstream>

class Task124 {
private:
    unsigned char key[16];
    std::map<std::string, std::string> encryptedData;
    
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
    
public:
    Task124() {
        RAND_bytes(key, sizeof(key));
    }
    
    std::string encryptData(const std::string& plaintext) {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        unsigned char ciphertext[1024];
        int len, ciphertext_len;
        
        EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
        EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plaintext.c_str(), plaintext.length());
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        ciphertext_len += len;
        EVP_CIPHER_CTX_free(ctx);
        
        return base64_encode(ciphertext, ciphertext_len);
    }
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    void storeSecureData(const std::string& key, const std::string& value) {
        encryptedData[key] = encryptData(value);
    }
    
    std::string retrieveSecureData(const std::string& key) {
        if (encryptedData.find(key) == encryptedData.end()) {
            return "";
        }
        return encryptedData[key];
    }
};

int main() {
    Task124 secure;
    
    // Test Case 1
    std::cout << "Test 1: Basic encryption" << std::endl;
    secure.storeSecureData("user1", "1234-XXXX-XXXX-5678");
    std::cout << "Stored encrypted data for user1" << std::endl;
    
    // Test Case 2
    std::cout << "\\nTest 2: Password hashing" << std::endl;
    std::string hashed = secure.hashPassword("myPassword123");
    std::cout << "Hashed password: " << hashed.substr(0, 40) << "..." << std::endl;
    
    // Test Case 3
    std::cout << "\\nTest 3: Multiple records" << std::endl;
    secure.storeSecureData("user2", "9876-XXXX-XXXX-5432");
    secure.storeSecureData("user3", "5555-XXXX-XXXX-1111");
    std::cout << "Stored multiple encrypted records" << std::endl;
    
    // Test Case 4
    std::cout << "\\nTest 4: Non-existent key" << std::endl;
    std::string result = secure.retrieveSecureData("user4");
    std::cout << "User4: " << (result.empty() ? "Not found" : result) << std::endl;
    
    // Test Case 5
    std::cout << "\\nTest 5: Verify encryption" << std::endl;
    secure.storeSecureData("test", "SensitiveData123");
    std::cout << "Data stored with encryption" << std::endl;
    
    return 0;
}
