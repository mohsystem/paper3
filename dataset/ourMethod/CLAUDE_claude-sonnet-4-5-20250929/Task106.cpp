
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>

// RAII wrapper for EVP_PKEY
struct EVPKeyDeleter {
    void operator()(EVP_PKEY* key) const { if (key) EVP_PKEY_free(key); }
};
using EVPKeyPtr = std::unique_ptr<EVP_PKEY, EVPKeyDeleter>;

// RAII wrapper for EVP_PKEY_CTX
struct EVPKeyCtxDeleter {
    void operator()(EVP_PKEY_CTX* ctx) const { if (ctx) EVP_PKEY_CTX_free(ctx); }
};
using EVPKeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, EVPKeyCtxDeleter>;

// Generate RSA keypair using OpenSSL EVP (2048-bit minimum for security)
EVPKeyPtr generateRSAKeyPair(int bits = 2048) {
    if (bits < 2048) {
        throw std::invalid_argument("RSA key size must be at least 2048 bits");
    }
    
    // Use EVP API for key generation
    EVPKeyCtxPtr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }
    
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        throw std::runtime_error("Failed to initialize key generation");
    }
    
    // Set key size
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), bits) <= 0) {
        throw std::runtime_error("Failed to set RSA key size");
    }
    
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) {
        throw std::runtime_error("Failed to generate RSA keypair");
    }
    
    return EVPKeyPtr(pkey);
}

// Encrypt data using RSA with OAEP padding (CWE-780 mitigation)
// OAEP provides semantic security and prevents chosen ciphertext attacks
std::vector<uint8_t> rsaEncrypt(EVP_PKEY* publicKey, const std::vector<uint8_t>& plaintext) {
    if (!publicKey) {
        throw std::invalid_argument("Public key is null");
    }
    
    if (plaintext.empty()) {
        throw std::invalid_argument("Plaintext is empty");
    }
    
    // Validate plaintext size - RSA-OAEP can encrypt limited data
    // Max size = key_size_bytes - 2*hash_size - 2
    // For 2048-bit RSA with SHA-256: max = 256 - 2*32 - 2 = 190 bytes
    int keySize = EVP_PKEY_size(publicKey);
    if (keySize <= 0 || keySize > 8192) { // Reasonable upper bound check
        throw std::runtime_error("Invalid key size");
    }
    
    size_t maxPlaintextSize = static_cast<size_t>(keySize) - 2 * 32 - 2;
    if (plaintext.size() > maxPlaintextSize) {
        throw std::invalid_argument("Plaintext too large for RSA-OAEP encryption");
    }
    
    // Create encryption context with OAEP padding
    EVPKeyCtxPtr ctx(EVP_PKEY_CTX_new(publicKey, nullptr));
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }
    
    if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    // Use RSA OAEP padding for security (prevents padding oracle attacks)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
        throw std::runtime_error("Failed to set OAEP padding");
    }
    
    // Set OAEP hash function to SHA-256
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) <= 0) {
        throw std::runtime_error("Failed to set OAEP hash function");
    }
    
    // Determine ciphertext length
    size_t ciphertextLen = 0;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &ciphertextLen, plaintext.data(), plaintext.size()) <= 0) {
        throw std::runtime_error("Failed to determine ciphertext length");
    }
    
    if (ciphertextLen == 0 || ciphertextLen > 16384) { // Sanity check
        throw std::runtime_error("Invalid ciphertext length");
    }
    
    // Perform encryption
    std::vector<uint8_t> ciphertext(ciphertextLen);
    if (EVP_PKEY_encrypt(ctx.get(), ciphertext.data(), &ciphertextLen, 
                         plaintext.data(), plaintext.size()) <= 0) {
        throw std::runtime_error("Encryption failed");
    }
    
    ciphertext.resize(ciphertextLen);
    return ciphertext;
}

// Decrypt data using RSA with OAEP padding
std::vector<uint8_t> rsaDecrypt(EVP_PKEY* privateKey, const std::vector<uint8_t>& ciphertext) {
    if (!privateKey) {
        throw std::invalid_argument("Private key is null");
    }
    
    if (ciphertext.empty()) {
        throw std::invalid_argument("Ciphertext is empty");
    }
    
    // Validate ciphertext size matches key size
    int keySize = EVP_PKEY_size(privateKey);
    if (keySize <= 0 || keySize > 8192) {
        throw std::runtime_error("Invalid key size");
    }
    
    if (ciphertext.size() != static_cast<size_t>(keySize)) {
        throw std::invalid_argument("Invalid ciphertext size");
    }
    
    // Create decryption context
    EVPKeyCtxPtr ctx(EVP_PKEY_CTX_new(privateKey, nullptr));
    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context");
    }
    
    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) {
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    // Use RSA OAEP padding (must match encryption padding)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
        throw std::runtime_error("Failed to set OAEP padding");
    }
    
    // Set OAEP hash function to SHA-256 (must match encryption)
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx.get(), EVP_sha256()) <= 0) {
        throw std::runtime_error("Failed to set OAEP hash function");
    }
    
    // Determine plaintext length
    size_t plaintextLen = 0;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &plaintextLen, ciphertext.data(), ciphertext.size()) <= 0) {
        throw std::runtime_error("Failed to determine plaintext length");
    }
    
    if (plaintextLen == 0 || plaintextLen > 16384) { // Sanity check
        throw std::runtime_error("Invalid plaintext length");
    }
    
    // Perform decryption
    std::vector<uint8_t> plaintext(plaintextLen);
    if (EVP_PKEY_decrypt(ctx.get(), plaintext.data(), &plaintextLen, 
                         ciphertext.data(), ciphertext.size()) <= 0) {
        throw std::runtime_error("Decryption failed");
    }
    
    plaintext.resize(plaintextLen);
    return plaintext;
}

int main() {
    try {
        // Initialize OpenSSL
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        
        // Test case 1: Basic encryption and decryption
        std::cout << "Test 1: Basic encryption/decryption" << std::endl;
        {
            EVPKeyPtr keypair = generateRSAKeyPair(2048);
            std::string message = "Hello, RSA with OAEP!";
            std::vector<uint8_t> plaintext(message.begin(), message.end());
            
            std::vector<uint8_t> ciphertext = rsaEncrypt(keypair.get(), plaintext);
            std::vector<uint8_t> decrypted = rsaDecrypt(keypair.get(), ciphertext);
            
            std::string result(decrypted.begin(), decrypted.end());
            std::cout << "Original: " << message << std::endl;
            std::cout << "Decrypted: " << result << std::endl;
            std::cout << "Match: " << (message == result ? "YES" : "NO") << std::endl << std::endl;
        }
        
        // Test case 2: Encrypt binary data
        std::cout << "Test 2: Binary data encryption" << std::endl;
        {
            EVPKeyPtr keypair = generateRSAKeyPair(2048);
            std::vector<uint8_t> binaryData = {0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE, 0xFD};
            
            std::vector<uint8_t> ciphertext = rsaEncrypt(keypair.get(), binaryData);
            std::vector<uint8_t> decrypted = rsaDecrypt(keypair.get(), ciphertext);
            
            bool match = (binaryData == decrypted);
            std::cout << "Binary data match: " << (match ? "YES" : "NO") << std::endl << std::endl;
        }
        
        // Test case 3: Maximum plaintext size
        std::cout << "Test 3: Maximum plaintext size" << std::endl;
        {
            EVPKeyPtr keypair = generateRSAKeyPair(2048);
            // For 2048-bit RSA with OAEP-SHA256: max = 256 - 2*32 - 2 = 190 bytes
            std::vector<uint8_t> maxPlaintext(190, 0xAA);
            
            std::vector<uint8_t> ciphertext = rsaEncrypt(keypair.get(), maxPlaintext);
            std::vector<uint8_t> decrypted = rsaDecrypt(keypair.get(), ciphertext);
            
            bool match = (maxPlaintext == decrypted);
            std::cout << "Max size data match: " << (match ? "YES" : "NO") << std::endl << std::endl;
        }
        
        // Test case 4: Different key sizes
        std::cout << "Test 4: 3072-bit RSA key" << std::endl;
        {
            EVPKeyPtr keypair = generateRSAKeyPair(3072);
            std::string message = "Testing 3072-bit RSA";
            std::vector<uint8_t> plaintext(message.begin(), message.end());
            
            std::vector<uint8_t> ciphertext = rsaEncrypt(keypair.get(), plaintext);
            std::vector<uint8_t> decrypted = rsaDecrypt(keypair.get(), ciphertext);
            
            std::string result(decrypted.begin(), decrypted.end());
            std::cout << "3072-bit match: " << (message == result ? "YES" : "NO") << std::endl << std::endl;
        }
        
        // Test case 5: Error handling - plaintext too large
        std::cout << "Test 5: Error handling" << std::endl;
        {
            EVPKeyPtr keypair = generateRSAKeyPair(2048);
            std::vector<uint8_t> tooLarge(300, 0xBB); // Too large for 2048-bit RSA-OAEP
            
            try {
                std::vector<uint8_t> ciphertext = rsaEncrypt(keypair.get(), tooLarge);
                std::cout << "ERROR: Should have thrown exception" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Caught expected exception: " << e.what() << std::endl;
            }
        }
        
        // Clean up OpenSSL
        EVP_cleanup();
        ERR_free_strings();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
