#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <iomanip>
#include <sstream>

// This code requires the OpenSSL library.
// To compile: g++ your_file.cpp -o your_app -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Helper to clean up OpenSSL's EVP_CIPHER_CTX
struct EvpCipherCtxDeleter {
    void operator()(EVP_CIPHER_CTX* ptr) const {
        if (ptr) {
            EVP_CIPHER_CTX_free(ptr);
        }
    }
};
using EvpCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, EvpCipherCtxDeleter>;

/**
 * A secure in-memory data store for sensitive information.
 * This class encrypts data using AES-256-GCM with a key derived from a password using PBKDF2.
 * WARNING: This is an example for demonstration purposes. Real-world secure storage
 * requires careful key management, platform-specific protections, and threat modeling.
 * Do not hardcode passwords in production applications.
 */
class SecureDataStore {
public:
    // Store data as a raw byte string
    void storeData(const std::string& id, const std::string& data, const std::string& password) {
        std::vector<unsigned char> salt(SALT_SIZE);
        if (RAND_bytes(salt.data(), salt.size()) != 1) {
            throw std::runtime_error("Failed to generate salt");
        }

        std::vector<unsigned char> key(KEY_SIZE);
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(),
                               PBKDF2_ITERATIONS, EVP_sha256(), key.size(), key.data()) != 1) {
            throw std::runtime_error("Failed to derive key");
        }

        std::vector<unsigned char> iv(IV_SIZE);
        if (RAND_bytes(iv.data(), iv.size()) != 1) {
            throw std::runtime_error("Failed to generate IV");
        }
        
        EvpCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
        if (!ctx) throw std::runtime_error("Failed to create cipher context");

        // Encrypt setup
        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1)
            throw std::runtime_error("Failed to set IV length");
        if (EVP_EncryptInit_ex(ctx.get(), NULL, NULL, key.data(), iv.data()) != 1)
            throw std::runtime_error("EVP_EncryptInit_ex key/iv failed");

        // Encrypt data
        std::vector<unsigned char> ciphertext(data.length());
        int len;
        if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, 
                              reinterpret_cast<const unsigned char*>(data.c_str()), data.length()) != 1) {
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        int ciphertext_len = len;

        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1)
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);

        // Get the GCM tag
        std::vector<unsigned char> tag(TAG_SIZE);
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data()) != 1)
            throw std::runtime_error("Failed to get GCM tag");
        
        // Combine salt + iv + tag + ciphertext for storage
        std::string stored_data;
        stored_data.append(reinterpret_cast<const char*>(salt.data()), salt.size());
        stored_data.append(reinterpret_cast<const char*>(iv.data()), iv.size());
        stored_data.append(reinterpret_cast<const char*>(tag.data()), tag.size());
        stored_data.append(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        
        vault_[id] = stored_data;
    }

    // Return empty string on failure
    std::string retrieveData(const std::string& id, const std::string& password) {
        if (vault_.find(id) == vault_.end()) {
            return "";
        }
        const std::string& stored_data = vault_.at(id);

        try {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(stored_data.c_str());
            
            // Extract components
            std::vector<unsigned char> salt(p, p + SALT_SIZE);
            p += SALT_SIZE;
            std::vector<unsigned char> iv(p, p + IV_SIZE);
            p += IV_SIZE;
            std::vector<unsigned char> tag(p, p + TAG_SIZE);
            p += TAG_SIZE;
            std::vector<unsigned char> ciphertext(p, p + (stored_data.length() - SALT_SIZE - IV_SIZE - TAG_SIZE));
            
            // Re-derive key
            std::vector<unsigned char> key(KEY_SIZE);
            if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(),
                                   PBKDF2_ITERATIONS, EVP_sha256(), key.size(), key.data()) != 1) {
                return ""; // Key derivation failed
            }

            EvpCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
            if (!ctx) return "";

            // Decrypt setup
            if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return "";
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1) return "";
            if (EVP_DecryptInit_ex(ctx.get(), NULL, NULL, key.data(), iv.data()) != 1) return "";
            
            // Decrypt data
            std::vector<unsigned char> plaintext(ciphertext.size());
            int len;
            if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) return "";
            int plaintext_len = len;

            // Set the expected tag. This must be done *before* EVP_DecryptFinal_ex.
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data()) != 1) return "";
            
            // Finalize decryption. This step verifies the tag. If it fails, the password was wrong or data was tampered with.
            if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
                return ""; // Authentication failed
            }
            plaintext_len += len;
            plaintext.resize(plaintext_len);

            return std::string(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        } catch (...) {
            return ""; // Catch any other exceptions
        }
    }

private:
    // Using std::string to hold raw bytes.
    std::map<std::string, std::string> vault_;
    
    // Cryptographic parameters
    static constexpr int SALT_SIZE = 16;
    static constexpr int IV_SIZE = 12;
    static constexpr int TAG_SIZE = 16;
    static constexpr int KEY_SIZE = 32; // 256 bits
    static constexpr int PBKDF2_ITERATIONS = 65536;
};

int main() {
    SecureDataStore store;
    std::string masterPassword = "aVery!Strong_Password123";

    std::cout << "--- Test Case 1: Storing multiple sensitive data entries ---" << std::endl;
    try {
        store.storeData("CreditCard_1", "1111-2222-3333-4444", masterPassword);
        store.storeData("SSN_PersonA", "987-65-4321", masterPassword);
        store.storeData("Email_Password", "S3cr3tP@ssw0rd!", masterPassword);
        store.storeData("API_Key_Prod", "xyz-abc-def-ghi-jkl-mno", masterPassword);
        store.storeData("MedicalRecord_ID", "MRN-555-01-1234", masterPassword);
        std::cout << "5 entries stored successfully." << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during storage: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "--- Test Case 2: Retrieving one entry with the correct password ---" << std::endl;
    std::string creditCard = store.retrieveData("CreditCard_1", masterPassword);
    std::cout << "Retrieved Credit Card: " << creditCard << std::endl;
    if (creditCard != "1111-2222-3333-4444") { std::cerr << "Test 2 FAILED" << std::endl; return 1; }
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Attempting to retrieve an entry with the WRONG password ---" << std::endl;
    std::string failedRetrieval = store.retrieveData("SSN_PersonA", "wrongPassword");
    std::cout << "Result of retrieval with wrong password: " << (failedRetrieval.empty() ? "[empty string]" : failedRetrieval) << std::endl;
    if (!failedRetrieval.empty()) { std::cerr << "Test 3 FAILED" << std::endl; return 1; }
    std::cout << "As expected, retrieval failed (returned empty string)." << std::endl << std::endl;
    
    std::cout << "--- Test Case 4: Attempting to retrieve a non-existent entry ---" << std::endl;
    std::string nonExistent = store.retrieveData("NonExistentKey", masterPassword);
    std::cout << "Result of retrieving non-existent key: " << (nonExistent.empty() ? "[empty string]" : nonExistent) << std::endl;
    if (!nonExistent.empty()) { std::cerr << "Test 4 FAILED" << std::endl; return 1; }
    std::cout << "As expected, retrieval failed (returned empty string)." << std::endl << std::endl;
    
    std::cout << "--- Test Case 5: Retrieving all stored entries successfully ---" << std::endl;
    std::cout << "API Key: " << store.retrieveData("API_Key_Prod", masterPassword) << std::endl;
    std::cout << "Email Password: " << store.retrieveData("Email_Password", masterPassword) << std::endl;
    std::cout << "SSN: " << store.retrieveData("SSN_PersonA", masterPassword) << std::endl;
    std::cout << "Medical Record: " << store.retrieveData("MedicalRecord_ID", masterPassword) << std::endl;
    std::cout << "All tests completed." << std::endl;
    
    return 0;
}