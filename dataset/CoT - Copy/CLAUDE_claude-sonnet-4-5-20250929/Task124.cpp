
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class Task124 {
private:
    std::vector<unsigned char> key;
    std::map<std::string, std::string> encryptedStorage;
    static const int KEY_SIZE = 32;
    static const int IV_SIZE = 12;
    static const int TAG_SIZE = 16;
    
    std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char* base64Chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64Chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }
    
    std::vector<unsigned char> base64Decode(const std::string& encoded) {
        static const std::string base64Chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> result;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[base64Chars[i]] = i;
        
        int val = 0;
        int valb = -8;
        for (unsigned char c : encoded) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }
        return result;
    }
    
public:
    Task124() {
        key.resize(KEY_SIZE);
        if (RAND_bytes(key.data(), KEY_SIZE) != 1) {
            throw std::runtime_error("Failed to generate encryption key");
        }
    }
    
    std::string encryptData(const std::string& plainText) {
        if (plainText.empty()) {
            throw std::invalid_argument("Data cannot be empty");
        }
        
        std::vector<unsigned char> iv(IV_SIZE);
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
            throw std::runtime_error("Failed to generate IV");
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        std::vector<unsigned char> ciphertext(plainText.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
            reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        int ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);
        
        std::vector<unsigned char> tag(TAG_SIZE);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to get tag");
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        result.insert(result.end(), tag.begin(), tag.end());
        
        return base64Encode(result);
    }
    
    std::string decryptData(const std::string& encryptedText) {
        if (encryptedText.empty()) {
            throw std::invalid_argument("Encrypted data cannot be empty");
        }
        
        std::vector<unsigned char> data = base64Decode(encryptedText);
        
        if (data.size() < IV_SIZE + TAG_SIZE) {
            throw std::runtime_error("Invalid encrypted data");
        }
        
        std::vector<unsigned char> iv(data.begin(), data.begin() + IV_SIZE);
        std::vector<unsigned char> tag(data.end() - TAG_SIZE, data.end());
        std::vector<unsigned char> ciphertext(data.begin() + IV_SIZE, data.end() - TAG_SIZE);
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
        
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        
        std::vector<unsigned char> plaintext(ciphertext.size());
        int len;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption failed");
        }
        int plaintext_len = len;
        
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set tag");
        }
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption verification failed");
        }
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        
        EVP_CIPHER_CTX_free(ctx);
        
        return std::string(plaintext.begin(), plaintext.end());
    }
    
    void storeSensitiveData(const std::string& key, const std::string& data) {
        if (key.empty()) {
            throw std::invalid_argument("Key cannot be empty");
        }
        std::string encryptedData = encryptData(data);
        encryptedStorage[key] = encryptedData;
    }
    
    std::string retrieveSensitiveData(const std::string& key) {
        if (key.empty()) {
            throw std::invalid_argument("Key cannot be empty");
        }
        auto it = encryptedStorage.find(key);
        if (it == encryptedStorage.end()) {
            return "";
        }
        return decryptData(it->second);
    }
};

int main() {
    try {
        Task124 secureStorage;
        
        // Test Case 1: Store and retrieve credit card number
        std::cout << "Test Case 1: Credit Card Storage" << std::endl;
        secureStorage.storeSensitiveData("creditCard1", "4532-1234-5678-9010");
        std::string retrievedCC = secureStorage.retrieveSensitiveData("creditCard1");
        std::cout << "Retrieved: " << retrievedCC << std::endl << std::endl;
        
        // Test Case 2: Store and retrieve personal information
        std::cout << "Test Case 2: Personal Information" << std::endl;
        secureStorage.storeSensitiveData("ssn1", "123-45-6789");
        std::string retrievedSSN = secureStorage.retrieveSensitiveData("ssn1");
        std::cout << "Retrieved: " << retrievedSSN << std::endl << std::endl;
        
        // Test Case 3: Store and retrieve email
        std::cout << "Test Case 3: Email Storage" << std::endl;
        secureStorage.storeSensitiveData("email1", "user@example.com");
        std::string retrievedEmail = secureStorage.retrieveSensitiveData("email1");
        std::cout << "Retrieved: " << retrievedEmail << std::endl << std::endl;
        
        // Test Case 4: Store and retrieve medical records
        std::cout << "Test Case 4: Medical Records" << std::endl;
        secureStorage.storeSensitiveData("medical1", "Patient: John Doe, Condition: Confidential");
        std::string retrievedMedical = secureStorage.retrieveSensitiveData("medical1");
        std::cout << "Retrieved: " << retrievedMedical << std::endl << std::endl;
        
        // Test Case 5: Retrieve non-existent data
        std::cout << "Test Case 5: Non-existent Key" << std::endl;
        std::string nonExistent = secureStorage.retrieveSensitiveData("nonExistent");
        std::cout << "Retrieved: " << (nonExistent.empty() ? "NULL" : nonExistent) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
