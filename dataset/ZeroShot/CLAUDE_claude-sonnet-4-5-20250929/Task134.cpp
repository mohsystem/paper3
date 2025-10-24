
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class Task134 {
private:
    std::map<std::string, std::vector<unsigned char>> keyStore;
    static const int KEY_SIZE = 32;  // 256 bits
    static const int IV_SIZE = 12;   // 96 bits for GCM
    static const int TAG_SIZE = 16;  // 128 bits
    
    std::string toBase64(const std::vector<unsigned char>& data) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        int in_len = data.size();
        const unsigned char* bytes_to_encode = data.data();
        
        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
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
                ret += base64_chars[char_array_4[j]];
            
            while((i++ < 3))
                ret += '=';
        }
        
        return ret;
    }
    
    std::vector<unsigned char> fromBase64(const std::string& encoded_string) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<unsigned char> ret;
        
        while (in_len-- && (encoded_string[in_] != '=') && 
               (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;
            
            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            
            for (j = 0; j < i - 1; j++)
                ret.push_back(char_array_3[j]);
        }
        
        return ret;
    }
    
public:
    Task134() {
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
    }
    
    std::string generateKey(const std::string& keyId) {
        std::vector<unsigned char> key(KEY_SIZE);
        if (RAND_bytes(key.data(), KEY_SIZE) != 1) {
            return "Error generating key";
        }
        keyStore[keyId] = key;
        return "Key generated successfully: " + keyId;
    }
    
    std::string encrypt(const std::string& keyId, const std::string& plaintext) {
        if (keyStore.find(keyId) == keyStore.end()) {
            return "Key not found: " + keyId;
        }
        
        std::vector<unsigned char>& key = keyStore[keyId];
        std::vector<unsigned char> iv(IV_SIZE);
        
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
            return "Error generating IV";
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return "Error creating context";
        
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error initializing encryption";
        }
        
        std::vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                              reinterpret_cast<const unsigned char*>(plaintext.c_str()), 
                              plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error encrypting";
        }
        
        int ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error finalizing encryption";
        }
        
        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);
        
        std::vector<unsigned char> tag(TAG_SIZE);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error getting tag";
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
        std::vector<unsigned char> combined;
        combined.insert(combined.end(), iv.begin(), iv.end());
        combined.insert(combined.end(), ciphertext.begin(), ciphertext.end());
        combined.insert(combined.end(), tag.begin(), tag.end());
        
        return toBase64(combined);
    }
    
    std::string decrypt(const std::string& keyId, const std::string& encryptedData) {
        if (keyStore.find(keyId) == keyStore.end()) {
            return "Key not found: " + keyId;
        }
        
        std::vector<unsigned char>& key = keyStore[keyId];
        std::vector<unsigned char> combined = fromBase64(encryptedData);
        
        if (combined.size() < IV_SIZE + TAG_SIZE) {
            return "Invalid encrypted data";
        }
        
        std::vector<unsigned char> iv(combined.begin(), combined.begin() + IV_SIZE);
        std::vector<unsigned char> tag(combined.end() - TAG_SIZE, combined.end());
        std::vector<unsigned char> ciphertext(combined.begin() + IV_SIZE, combined.end() - TAG_SIZE);
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return "Error creating context";
        
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error initializing decryption";
        }
        
        std::vector<unsigned char> plaintext(ciphertext.size());
        int len;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error decrypting";
        }
        
        int plaintext_len = len;
        
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error setting tag";
        }
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return "Error finalizing decryption";
        }
        
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        
        EVP_CIPHER_CTX_free(ctx);
        
        return std::string(plaintext.begin(), plaintext.end());
    }
    
    std::string deleteKey(const std::string& keyId) {
        if (keyStore.find(keyId) != keyStore.end()) {
            keyStore.erase(keyId);
            return "Key deleted: " + keyId;
        }
        return "Key not found: " + keyId;
    }
    
    std::vector<std::string> listKeys() {
        std::vector<std::string> keys;
        for (const auto& pair : keyStore) {
            keys.push_back(pair.first);
        }
        return keys;
    }
};

int main() {
    Task134 keyManager;
    
    // Test Case 1: Generate a key
    std::cout << "Test 1 - Generate Key:" << std::endl;
    std::cout << keyManager.generateKey("key1") << std::endl << std::endl;
    
    // Test Case 2: Encrypt data
    std::cout << "Test 2 - Encrypt Data:" << std::endl;
    std::string encrypted = keyManager.encrypt("key1", "Hello, Secure World!");
    std::cout << "Encrypted: " << encrypted << std::endl << std::endl;
    
    // Test Case 3: Decrypt data
    std::cout << "Test 3 - Decrypt Data:" << std::endl;
    std::string decrypted = keyManager.decrypt("key1", encrypted);
    std::cout << "Decrypted: " << decrypted << std::endl << std::endl;
    
    // Test Case 4: List all keys
    std::cout << "Test 4 - List Keys:" << std::endl;
    keyManager.generateKey("key2");
    keyManager.generateKey("key3");
    std::vector<std::string> keys = keyManager.listKeys();
    std::cout << "Keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl << std::endl;
    
    // Test Case 5: Delete a key
    std::cout << "Test 5 - Delete Key:" << std::endl;
    std::cout << keyManager.deleteKey("key2") << std::endl;
    keys = keyManager.listKeys();
    std::cout << "Remaining keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
