
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/rand.h>

class Task134 {
private:
    std::map<std::string, std::string> keyStore;
    
    std::string bytesToHex(const unsigned char* data, size_t len) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }
    
    std::vector<unsigned char> hexToBytes(const std::string& hex) {
        std::vector<unsigned char> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }
    
public:
    Task134() {}
    
    std::string generateKey(const std::string& keyId, int keySize) {
        try {
            int keySizeBytes = keySize / 8;
            unsigned char key[keySizeBytes];
            
            if (RAND_bytes(key, keySizeBytes) != 1) {
                return "Error: Failed to generate random key";
            }
            
            std::string encodedKey = bytesToHex(key, keySizeBytes);
            keyStore[keyId] = encodedKey;
            return encodedKey;
        } catch (...) {
            return "Error: Exception occurred";
        }
    }
    
    std::string getKey(const std::string& keyId) {
        auto it = keyStore.find(keyId);
        if (it != keyStore.end()) {
            return it->second;
        }
        return "Key not found";
    }
    
    bool deleteKey(const std::string& keyId) {
        auto it = keyStore.find(keyId);
        if (it != keyStore.end()) {
            keyStore.erase(it);
            return true;
        }
        return false;
    }
    
    std::vector<std::string> listKeys() {
        std::vector<std::string> keys;
        for (const auto& pair : keyStore) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    std::string rotateKey(const std::string& keyId, int keySize) {
        if (keyStore.find(keyId) != keyStore.end()) {
            return generateKey(keyId, keySize);
        }
        return "Key not found";
    }
    
    std::string encryptData(const std::string& keyId, const std::string& data) {
        auto it = keyStore.find(keyId);
        if (it == keyStore.end()) {
            return "Key not found";
        }
        
        std::vector<unsigned char> key = hexToBytes(it->second);
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_bytes(iv, AES_BLOCK_SIZE);
        
        // Simple XOR encryption for demonstration (not secure for production)
        std::string result = bytesToHex(iv, AES_BLOCK_SIZE);
        for (size_t i = 0; i < data.length(); ++i) {
            unsigned char encrypted = data[i] ^ key[i % key.size()];
            result += bytesToHex(&encrypted, 1);
        }
        
        return result;
    }
    
    std::string decryptData(const std::string& keyId, const std::string& encryptedData) {
        auto it = keyStore.find(keyId);
        if (it == keyStore.end()) {
            return "Key not found";
        }
        
        std::vector<unsigned char> key = hexToBytes(it->second);
        std::vector<unsigned char> encrypted = hexToBytes(encryptedData.substr(32));
        
        std::string result;
        for (size_t i = 0; i < encrypted.size(); ++i) {
            char decrypted = encrypted[i] ^ key[i % key.size()];
            result += decrypted;
        }
        
        return result;
    }
};

int main() {
    Task134 keyManager;
    
    std::cout << "Test Case 1: Generate and retrieve keys" << std::endl;
    std::string key1 = keyManager.generateKey("key1", 128);
    std::cout << "Generated key1: " << key1 << std::endl;
    std::cout << "Retrieved key1: " << keyManager.getKey("key1") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: List all keys" << std::endl;
    keyManager.generateKey("key2", 256);
    keyManager.generateKey("key3", 128);
    std::vector<std::string> keys = keyManager.listKeys();
    std::cout << "All keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl << std::endl;
    
    std::cout << "Test Case 3: Encrypt and decrypt data" << std::endl;
    std::string originalData = "Hello, World!";
    std::string encrypted = keyManager.encryptData("key1", originalData);
    std::cout << "Encrypted: " << encrypted << std::endl;
    std::string decrypted = keyManager.decryptData("key1", encrypted);
    std::cout << "Decrypted: " << decrypted << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Rotate key" << std::endl;
    std::string oldKey = keyManager.getKey("key2");
    std::cout << "Old key2: " << oldKey << std::endl;
    std::string newKey = keyManager.rotateKey("key2", 256);
    std::cout << "New key2: " << newKey << std::endl;
    std::cout << "Keys are different: " << (oldKey != newKey ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Delete key" << std::endl;
    std::cout << "Delete key3: " << (keyManager.deleteKey("key3") ? "true" : "false") << std::endl;
    keys = keyManager.listKeys();
    std::cout << "Remaining keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    std::cout << "Try to get deleted key: " << keyManager.getKey("key3") << std::endl;
    
    return 0;
}
