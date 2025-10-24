
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

class Task134 {
private:
    static const int AES_KEY_SIZE = 32;
    static const int RSA_KEY_SIZE = 2048;
    static const int GCM_IV_LENGTH = 12;
    static const int GCM_TAG_LENGTH = 16;
    static const int PBKDF2_ITERATIONS = 100000;
    
    std::map<std::string, std::vector<unsigned char>> keyStore;
    std::vector<unsigned char> masterKey;
    
    std::vector<unsigned char> deriveMasterKey(const std::string& password) {
        std::vector<unsigned char> salt(32);
        RAND_bytes(salt.data(), 32);
        
        std::vector<unsigned char> key(AES_KEY_SIZE);
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          AES_KEY_SIZE, key.data());
        
        return key;
    }
    
    std::vector<unsigned char> encryptKey(const std::vector<unsigned char>& keyBytes) {
        std::vector<unsigned char> iv(GCM_IV_LENGTH);
        RAND_bytes(iv.data(), GCM_IV_LENGTH);
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return std::vector<unsigned char>();
        
        std::vector<unsigned char> ciphertext(keyBytes.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        std::vector<unsigned char> tag(GCM_TAG_LENGTH);
        
        int len;
        int ciphertext_len;
        
        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, masterKey.data(), iv.data())) {
            EVP_CIPHER_CTX_free(ctx);
            return std::vector<unsigned char>();
        }
        
        if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, keyBytes.data(), keyBytes.size())) {
            EVP_CIPHER_CTX_free(ctx);
            return std::vector<unsigned char>();
        }
        ciphertext_len = len;
        
        if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
            EVP_CIPHER_CTX_free(ctx);
            return std::vector<unsigned char>();
        }
        ciphertext_len += len;
        
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag.data())) {
            EVP_CIPHER_CTX_free(ctx);
            return std::vector<unsigned char>();
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);
        result.insert(result.end(), tag.begin(), tag.end());
        
        return result;
    }
    
public:
    Task134(const std::string& masterPassword) {
        masterKey = deriveMasterKey(masterPassword);
    }
    
    std::string generateAESKey(const std::string& keyId) {
        if (keyId.empty()) {
            throw std::invalid_argument("Key ID cannot be empty");
        }
        
        std::vector<unsigned char> key(AES_KEY_SIZE);
        RAND_bytes(key.data(), AES_KEY_SIZE);
        
        std::vector<unsigned char> encryptedKey = encryptKey(key);
        keyStore[keyId] = encryptedKey;
        
        return keyId;
    }
    
    std::string generateRSAKeyPair(const std::string& keyId) {
        if (keyId.empty()) {
            throw std::invalid_argument("Key ID cannot be empty");
        }
        
        EVP_PKEY* pkey = EVP_PKEY_new();
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
        
        EVP_PKEY_keygen_init(ctx);
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_SIZE);
        EVP_PKEY_keygen(ctx, &pkey);
        
        BIO* priv_bio = BIO_new(BIO_s_mem());
        BIO* pub_bio = BIO_new(BIO_s_mem());
        
        PEM_write_bio_PrivateKey(priv_bio, pkey, NULL, NULL, 0, NULL, NULL);
        PEM_write_bio_PUBKEY(pub_bio, pkey);
        
        char* priv_data;
        char* pub_data;
        long priv_len = BIO_get_mem_data(priv_bio, &priv_data);
        long pub_len = BIO_get_mem_data(pub_bio, &pub_data);
        
        std::vector<unsigned char> privKey(priv_data, priv_data + priv_len);
        std::vector<unsigned char> pubKey(pub_data, pub_data + pub_len);
        
        keyStore[keyId + "_private"] = encryptKey(privKey);
        keyStore[keyId + "_public"] = encryptKey(pubKey);
        
        BIO_free(priv_bio);
        BIO_free(pub_bio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        
        return keyId;
    }
    
    bool deleteKey(const std::string& keyId) {
        return keyStore.erase(keyId) > 0;
    }
    
    bool keyExists(const std::string& keyId) {
        return keyStore.find(keyId) != keyStore.end();
    }
    
    std::set<std::string> listKeys() {
        std::set<std::string> keys;
        for (const auto& pair : keyStore) {
            keys.insert(pair.first);
        }
        return keys;
    }
    
    void clearAllKeys() {
        keyStore.clear();
    }
};

int main() {
    try {
        std::cout << "Test Case 1: Initialize Key Manager" << std::endl;
        Task134 keyManager("SecurePassword123!@#");
        std::cout << "Key manager initialized successfully" << std::endl << std::endl;
        
        std::cout << "Test Case 2: Generate AES Keys" << std::endl;
        std::string aesKey1 = keyManager.generateAESKey("aes-key-001");
        std::string aesKey2 = keyManager.generateAESKey("aes-key-002");
        std::cout << "Generated AES keys: " << aesKey1 << ", " << aesKey2 << std::endl;
        std::cout << "Key exists (aes-key-001): " << keyManager.keyExists("aes-key-001") << std::endl << std::endl;
        
        std::cout << "Test Case 3: Generate RSA Key Pairs" << std::endl;
        std::string rsaKey1 = keyManager.generateRSAKeyPair("rsa-keypair-001");
        std::cout << "Generated RSA key pair: " << rsaKey1 << std::endl;
        std::cout << "Private key exists: " << keyManager.keyExists("rsa-keypair-001_private") << std::endl;
        std::cout << "Public key exists: " << keyManager.keyExists("rsa-keypair-001_public") << std::endl << std::endl;
        
        std::cout << "Test Case 4: List All Keys" << std::endl;
        std::set<std::string> allKeys = keyManager.listKeys();
        std::cout << "Total keys stored: " << allKeys.size() << std::endl;
        std::cout << "Keys: ";
        for (const auto& key : allKeys) {
            std::cout << key << " ";
        }
        std::cout << std::endl << std::endl;
        
        std::cout << "Test Case 5: Delete Keys" << std::endl;
        bool deleted = keyManager.deleteKey("aes-key-001");
        std::cout << "Deleted aes-key-001: " << deleted << std::endl;
        std::cout << "Key exists after deletion: " << keyManager.keyExists("aes-key-001") << std::endl;
        std::cout << "Remaining keys: " << keyManager.listKeys().size() << std::endl;
        
        keyManager.clearAllKeys();
        std::cout << "All keys cleared. Remaining keys: " << keyManager.listKeys().size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
