
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>

// Security constants
constexpr size_t SALT_LENGTH = 16;
constexpr size_t IV_LENGTH = 16;
constexpr size_t HMAC_LENGTH = 32;
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr size_t AES_KEY_LENGTH = 32;  // 256 bits
constexpr size_t MAX_PLAINTEXT_SIZE = 10 * 1024 * 1024;  // 10MB

/**
 * RAII wrapper for secure memory clearing
 */
class SecureBuffer {
private:
    std::vector<unsigned char> data;
    
public:
    explicit SecureBuffer(size_t size) : data(size, 0) {}
    
    ~SecureBuffer() {
        // Use OPENSSL_cleanse for secure memory clearing
        if (!data.empty()) {
            OPENSSL_cleanse(data.data(), data.size());
        }
    }
    
    unsigned char* get() { return data.data(); }
    const unsigned char* get() const { return data.data(); }
    size_t size() const { return data.size(); }
    
    // Prevent copying
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
};

/**
 * Derives a 256-bit encryption key from a passphrase using PBKDF2-HMAC-SHA256.
 * Uses 210,000 iterations as per security requirements.
 */
bool deriveKey(const std::string& passphrase, const unsigned char* salt, 
               size_t saltLen, unsigned char* key, size_t keyLen) {
    // Validate inputs - treat all inputs as untrusted
    if (passphrase.empty()) {
        throw std::invalid_argument("Passphrase must not be empty");
    }
    if (salt == nullptr || saltLen != SALT_LENGTH) {
        throw std::invalid_argument("Salt must be exactly 16 bytes");
    }
    if (key == nullptr || keyLen != AES_KEY_LENGTH) {
        throw std::invalid_argument("Invalid key buffer");
    }
    
    // Use PBKDF2 with HMAC-SHA256 for secure key derivation
    int result = PKCS5_PBKDF2_HMAC(
        passphrase.c_str(),
        static_cast<int>(passphrase.length()),
        salt,
        static_cast<int>(saltLen),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        static_cast<int>(keyLen),
        key
    );
    
    return result == 1;
}

/**
 * Computes HMAC-SHA256 for Encrypt-then-MAC pattern.
 */
bool computeHmac(const unsigned char* key, size_t keyLen,
                 const unsigned char* data, size_t dataLen,
                 unsigned char* hmac, unsigned int* hmacLen) {
    unsigned char* result = HMAC(
        EVP_sha256(),
        key,
        static_cast<int>(keyLen),
        data,
        dataLen,
        hmac,
        hmacLen
    );
    
    return result != nullptr && *hmacLen == HMAC_LENGTH;
}

/**
 * Encrypts data using AES-256-CBC with HMAC-SHA256 (Encrypt-then-MAC).
 * Returns a structured format: [magic][version][salt][hmac_salt][iv][ciphertext][hmac_tag]
 * 
 * Security measures:
 * - Uses AES-256 in CBC mode with PKCS7 padding
 * - Implements Encrypt-then-MAC pattern for authentication
 * - Generates unique 16-byte salt per encryption using OpenSSL RAND_bytes
 * - Generates unique 16-byte IV per encryption using CSPRNG
 * - Uses PBKDF2-HMAC-SHA256 with 210,000 iterations
 * - Validates all inputs before processing
 * - Uses RAII for secure memory management
 */
std::vector<unsigned char> encryptAesCbcWithHmac(
    const std::vector<unsigned char>& plaintext,
    const std::string& passphrase) {
    
    // Input validation - treat all inputs as untrusted
    if (plaintext.empty()) {
        throw std::invalid_argument("Plaintext must not be empty");
    }
    if (passphrase.empty()) {
        throw std::invalid_argument("Passphrase must not be empty");
    }
    if (plaintext.size() > MAX_PLAINTEXT_SIZE) {
        throw std::invalid_argument("Plaintext exceeds maximum size");
    }
    
    // Generate cryptographically secure random salt and IV
    unsigned char salt[SALT_LENGTH];
    unsigned char hmacSalt[SALT_LENGTH];
    unsigned char iv[IV_LENGTH];
    
    if (RAND_bytes(salt, SALT_LENGTH) != 1 ||
        RAND_bytes(hmacSalt, SALT_LENGTH) != 1 ||
        RAND_bytes(iv, IV_LENGTH) != 1) {
        throw std::runtime_error("Failed to generate random bytes");
    }
    
    // Derive encryption key using secure KDF
    SecureBuffer encryptionKey(AES_KEY_LENGTH);
    if (!deriveKey(passphrase, salt, SALT_LENGTH, encryptionKey.get(), AES_KEY_LENGTH)) {
        throw std::runtime_error("Failed to derive encryption key");
    }
    
    // Derive separate HMAC key for Encrypt-then-MAC
    SecureBuffer hmacKey(AES_KEY_LENGTH);
    if (!deriveKey(passphrase, hmacSalt, SALT_LENGTH, hmacKey.get(), AES_KEY_LENGTH)) {
        throw std::runtime_error("Failed to derive HMAC key");
    }
    
    // Create and initialize encryption context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    // Initialize AES-256-CBC encryption
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, 
                           encryptionKey.get(), iv) != 1) {
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    // Allocate buffer for ciphertext (includes padding)
    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0;
    int ciphertextLen = 0;
    
    // Encrypt the plaintext
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, 
                          plaintext.data(), static_cast<int>(plaintext.size())) != 1) {
        throw std::runtime_error("Encryption failed");
    }
    ciphertextLen = len;
    
    // Finalize encryption (handles padding)
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        throw std::runtime_error("Encryption finalization failed");
    }
    ciphertextLen += len;
    ciphertext.resize(ciphertextLen);
    
    // Build the output structure: magic + version + salt + hmac_salt + iv + ciphertext
    const char* magic = "ENC1";
    const unsigned char version = 1;
    
    std::vector<unsigned char> result;
    result.reserve(4 + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + ciphertextLen + HMAC_LENGTH);
    
    // Add magic
    result.insert(result.end(), magic, magic + 4);
    // Add version
    result.push_back(version);
    // Add salt
    result.insert(result.end(), salt, salt + SALT_LENGTH);
    // Add HMAC salt
    result.insert(result.end(), hmacSalt, hmacSalt + SALT_LENGTH);
    // Add IV
    result.insert(result.end(), iv, iv + IV_LENGTH);
    // Add ciphertext
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    
    // Compute HMAC over all data (Encrypt-then-MAC pattern)
    unsigned char hmacTag[HMAC_LENGTH];
    unsigned int hmacLen = 0;
    
    if (!computeHmac(hmacKey.get(), AES_KEY_LENGTH, 
                     result.data(), result.size(), hmacTag, &hmacLen)) {
        throw std::runtime_error("HMAC computation failed");
    }
    
    // Add HMAC tag
    result.insert(result.end(), hmacTag, hmacTag + HMAC_LENGTH);
    
    return result;
}

/**
 * Decrypts data encrypted with encryptAesCbcWithHmac.
 * Verifies HMAC before decryption (fail closed on authentication failure).
 */
std::vector<unsigned char> decryptAesCbcWithHmac(
    const std::vector<unsigned char>& encryptedData,
    const std::string& passphrase) {
    
    // Input validation
    if (encryptedData.empty()) {
        throw std::invalid_argument("Encrypted data must not be empty");
    }
    if (passphrase.empty()) {
        throw std::invalid_argument("Passphrase must not be empty");
    }
    
    // Minimum size check
    size_t minSize = 4 + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + HMAC_LENGTH;
    if (encryptedData.size() < minSize) {
        throw std::invalid_argument("Invalid encrypted data format");
    }
    
    size_t offset = 0;
    
    // Parse and validate magic
    const char* expectedMagic = "ENC1";
    if (std::memcmp(encryptedData.data() + offset, expectedMagic, 4) != 0) {
        throw std::invalid_argument("Invalid file format");
    }
    offset += 4;
    
    // Parse and validate version
    unsigned char version = encryptedData[offset++];
    if (version != 1) {
        throw std::invalid_argument("Unsupported version");
    }
    
    // Parse salt, hmac_salt, and IV
    const unsigned char* salt = encryptedData.data() + offset;
    offset += SALT_LENGTH;
    
    const unsigned char* hmacSalt = encryptedData.data() + offset;
    offset += SALT_LENGTH;
    
    const unsigned char* iv = encryptedData.data() + offset;
    offset += IV_LENGTH;
    
    // Parse ciphertext and HMAC tag
    size_t ciphertextLen = encryptedData.size() - offset - HMAC_LENGTH;
    const unsigned char* ciphertext = encryptedData.data() + offset;
    const unsigned char* hmacTag = encryptedData.data() + encryptedData.size() - HMAC_LENGTH;
    
    // Derive HMAC key to verify authentication tag
    SecureBuffer hmacKey(AES_KEY_LENGTH);
    if (!deriveKey(passphrase, hmacSalt, SALT_LENGTH, hmacKey.get(), AES_KEY_LENGTH)) {
        throw std::runtime_error("Failed to derive HMAC key");
    }
    
    // Verify HMAC tag using constant-time comparison
    unsigned char expectedHmac[HMAC_LENGTH];
    unsigned int hmacLen = 0;
    
    if (!computeHmac(hmacKey.get(), AES_KEY_LENGTH,
                     encryptedData.data(), encryptedData.size() - HMAC_LENGTH,
                     expectedHmac, &hmacLen)) {
        throw std::runtime_error("HMAC computation failed");
    }
    
    // Use constant-time comparison to prevent timing attacks
    if (CRYPTO_memcmp(expectedHmac, hmacTag, HMAC_LENGTH) != 0) {
        throw std::runtime_error("Authentication failed");
    }
    
    // HMAC verified - proceed with decryption
    SecureBuffer encryptionKey(AES_KEY_LENGTH);
    if (!deriveKey(passphrase, salt, SALT_LENGTH, encryptionKey.get(), AES_KEY_LENGTH)) {
        throw std::runtime_error("Failed to derive encryption key");
    }
    
    // Create and initialize decryption context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    // Initialize AES-256-CBC decryption
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr,
                           encryptionKey.get(), iv) != 1) {
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    // Allocate buffer for plaintext
    std::vector<unsigned char> plaintext(ciphertextLen + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0;
    int plaintextLen = 0;
    
    // Decrypt the ciphertext
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len,
                          ciphertext, static_cast<int>(ciphertextLen)) != 1) {
        throw std::runtime_error("Decryption failed");
    }
    plaintextLen = len;
    
    // Finalize decryption (handles padding removal)
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
        throw std::runtime_error("Decryption finalization failed");
    }
    plaintextLen += len;
    plaintext.resize(plaintextLen);
    
    return plaintext;
}

int main() {
    std::cout << "AES-256-CBC with HMAC-SHA256 Encryption Test Cases\\n\\n";
    
    try {
        // Test case 1: Basic encryption/decryption
        std::cout << "Test 1: Basic encryption/decryption\\n";
        std::string text1 = "Secret message to encrypt";
        std::vector<unsigned char> plaintext1(text1.begin(), text1.end());
        std::string passphrase1 = "StrongPassphrase123!@#";
        
        auto encrypted1 = encryptAesCbcWithHmac(plaintext1, passphrase1);
        auto decrypted1 = decryptAesCbcWithHmac(encrypted1, passphrase1);
        
        std::cout << "Original:  " << text1 << "\\n";
        std::cout << "Encrypted length: " << encrypted1.size() << " bytes\\n";
        std::cout << "Decrypted: " << std::string(decrypted1.begin(), decrypted1.end()) << "\\n";
        std::cout << "Match: " << (plaintext1 == decrypted1 ? "true" : "false") << "\\n\\n";
        
        // Test case 2: Encrypting binary data
        std::cout << "Test 2: Binary data encryption\\n";
        std::vector<unsigned char> plaintext2(64);
        if (RAND_bytes(plaintext2.data(), 64) != 1) {
            throw std::runtime_error("Failed to generate random data");
        }
        std::string passphrase2 = "AnotherSecurePass456$%^";
        
        auto encrypted2 = encryptAesCbcWithHmac(plaintext2, passphrase2);
        auto decrypted2 = decryptAesCbcWithHmac(encrypted2, passphrase2);
        
        std::cout << "Original length:  " << plaintext2.size() << " bytes\\n";
        std::cout << "Encrypted length: " << encrypted2.size() << " bytes\\n";
        std::cout << "Match: " << (plaintext2 == decrypted2 ? "true" : "false") << "\\n\\n";
        
        // Test case 3: UTF-8 text encryption
        std::cout << "Test 3: UTF-8 text encryption\\n";
        std::string text3 = "Hello, World! 🔐";
        std::vector<unsigned char> plaintext3(text3.begin(), text3.end());
        std::string passphrase3 = "Unicode_Passphrase_2024!";
        
        auto encrypted3 = encryptAesCbcWithHmac(plaintext3, passphrase3);
        auto decrypted3 = decryptAesCbcWithHmac(encrypted3, passphrase3);
        
        std::cout << "Original:  " << text3 << "\\n";
        std::cout << "Decrypted: " << std::string(decrypted3.begin(), decrypted3.end()) << "\\n";
        std::cout << "Match: " << (plaintext3 == decrypted3 ? "true" : "false") << "\\n\\n";
        
        // Test case 4: Large data encryption
        std::cout << "Test 4: Large data encryption\\n";
        std::vector<unsigned char> plaintext4(10000, 'X');
        std::string passphrase4 = "LargeDataPass789&*()";
        
        auto encrypted4 = encryptAesCbcWithHmac(plaintext4, passphrase4);
        auto decrypted4 = decryptAesCbcWithHmac(encrypted4, passphrase4);
        
        std::cout << "Original length:  " << plaintext4.size() << " bytes\\n";
        std::cout << "Encrypted length: " << encrypted4.size() << " bytes\\n";
        std::cout << "Match: " << (plaintext4 == decrypted4 ? "true" : "false") << "\\n\\n";
        
        // Test case 5: Verify HMAC authentication failure detection
        std::cout << "Test 5: HMAC authentication failure detection\\n";
        std::string text5 = "Authenticated message";
        std::vector<unsigned char> plaintext5(text5.begin(), text5.end());
        std::string passphrase5 = "AuthTestPass!@#123";
        
        auto encrypted5 = encryptAesCbcWithHmac(plaintext5, passphrase5);
        
        // Tamper with the ciphertext
        auto tampered = encrypted5;
        tampered[tampered.size() - 50] ^= 0xFF;
        
        try {
            decryptAesCbcWithHmac(tampered, passphrase5);
            std::cout << "FAILED: Tampered data was accepted\\n";
        } catch (const std::exception& e) {
            std::cout << "SUCCESS: Tampered data rejected - " << e.what() << "\\n\\n";
        }
        
        std::cout << "All test cases completed successfully!\\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << "\\n";
        return 1;
    }
    
    return 0;
}
