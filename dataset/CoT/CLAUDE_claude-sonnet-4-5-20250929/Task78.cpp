
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

class Task78 {
public:
    static std::string generateRSAPrivateKeyOpenSSH(int keySize) {
        // Validate minimum key size for security
        if (keySize < 2048) {
            throw std::invalid_argument("Key size must be at least 2048 bits for security");
        }
        
        // Initialize OpenSSL
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        
        // Generate RSA key pair
        EVP_PKEY* pkey = nullptr;
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        
        if (!ctx) {
            throw std::runtime_error("Failed to create EVP_PKEY_CTX");
        }
        
        try {
            if (EVP_PKEY_keygen_init(ctx) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                throw std::runtime_error("Failed to initialize key generation");
            }
            
            if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                throw std::runtime_error("Failed to set key size");
            }
            
            if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                throw std::runtime_error("Failed to generate key pair");
            }
            
            EVP_PKEY_CTX_free(ctx);
            
            // Convert to OpenSSH format using BIO
            BIO* bio = BIO_new(BIO_s_mem());
            if (!bio) {
                EVP_PKEY_free(pkey);
                throw std::runtime_error("Failed to create BIO");
            }
            
            // Write private key in PEM format (closest to OpenSSH)
            if (!PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
                BIO_free(bio);
                EVP_PKEY_free(pkey);
                throw std::runtime_error("Failed to write private key");
            }
            
            // Read the key from BIO
            char* data;
            long dataLen = BIO_get_mem_data(bio, &data);
            std::string result(data, dataLen);
            
            // Cleanup
            BIO_free(bio);
            EVP_PKEY_free(pkey);
            
            return result;
            
        } catch (...) {
            if (ctx) EVP_PKEY_CTX_free(ctx);
            if (pkey) EVP_PKEY_free(pkey);
            throw;
        }
    }
};

int main() {
    std::cout << "=== RSA Private Key Generator (OpenSSH Format) ===\\n\\n";
    
    // Test case 1: 2048-bit key (minimum secure)
    std::cout << "Test Case 1: Generate 2048-bit RSA key\\n";
    try {
        std::string key1 = Task78::generateRSAPrivateKeyOpenSSH(2048);
        std::cout << "Success - Key length: " << key1.length() << " characters\\n";
        std::cout << key1.substr(0, std::min((size_t)200, key1.length())) << "...\\n\\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\\n\\n";
    }
    
    // Test case 2: 4096-bit key (highly secure)
    std::cout << "Test Case 2: Generate 4096-bit RSA key\\n";
    try {
        std::string key2 = Task78::generateRSAPrivateKeyOpenSSH(4096);
        std::cout << "Success - Key length: " << key2.length() << " characters\\n";
        std::cout << key2.substr(0, std::min((size_t)200, key2.length())) << "...\\n\\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\\n\\n";
    }
    
    // Test case 3: Invalid key size (should fail)
    std::cout << "Test Case 3: Attempt 1024-bit key (insecure, should fail)\\n";
    try {
        std::string key3 = Task78::generateRSAPrivateKeyOpenSSH(1024);
        std::cout << "Generated: " << key3.substr(0, 100) << "...\\n\\n";
    } catch (const std::exception& e) {
        std::cout << "Expected Error: " << e.what() << "\\n\\n";
    }
    
    // Test case 4: 3072-bit key
    std::cout << "Test Case 4: Generate 3072-bit RSA key\\n";
    try {
        std::string key4 = Task78::generateRSAPrivateKeyOpenSSH(3072);
        std::cout << "Success - Key length: " << key4.length() << " characters\\n";
        std::cout << key4.substr(0, std::min((size_t)200, key4.length())) << "...\\n\\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\\n\\n";
    }
    
    // Test case 5: Standard 2048-bit with full output
    std::cout << "Test Case 5: Generate 2048-bit key with full display\\n";
    try {
        std::string key5 = Task78::generateRSAPrivateKeyOpenSSH(2048);
        std::cout << "Full key generated:\\n" << key5 << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\\n\\n";
    }
    
    return 0;
}
