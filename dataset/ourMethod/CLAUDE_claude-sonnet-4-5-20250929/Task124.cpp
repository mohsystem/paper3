
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>

// Constants for cryptographic operations
constexpr size_t SALT_SIZE = 16;
constexpr size_t IV_SIZE = 12;
constexpr size_t TAG_SIZE = 16;
constexpr size_t KEY_SIZE = 32;
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr char FILE_MAGIC[] = "ENC1";
constexpr uint8_t FILE_VERSION = 1;

// RAII wrapper for OpenSSL cipher context
class CipherContext {
    EVP_CIPHER_CTX* ctx;
public:
    CipherContext() : ctx(EVP_CIPHER_CTX_new()) {
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
    }
    ~CipherContext() { 
        if (ctx) {
            EVP_CIPHER_CTX_free(ctx);
        }
    }
    EVP_CIPHER_CTX* get() { return ctx; }
    CipherContext(const CipherContext&) = delete;
    CipherContext& operator=(const CipherContext&) = delete;
};

// Securely clear sensitive data from memory
void secureClear(void* ptr, size_t size) {
    if (ptr && size > 0) {
        OPENSSL_cleanse(ptr, size);
    }
}

// Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256
bool deriveKey(const std::string& passphrase, const std::vector<uint8_t>& salt, 
               std::vector<uint8_t>& key) {
    if (passphrase.empty() || salt.size() != SALT_SIZE) {
        return false;
    }
    
    key.resize(KEY_SIZE);
    
    // Use PBKDF2 with HMAC-SHA-256 for key derivation
    int result = PKCS5_PBKDF2_HMAC(
        passphrase.c_str(), static_cast<int>(passphrase.length()),
        salt.data(), static_cast<int>(salt.size()),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        static_cast<int>(key.size()), key.data()
    );
    
    return result == 1;
}

// Encrypt sensitive data using AES-256-GCM
bool encryptData(const std::string& plaintext, const std::string& passphrase,
                 std::vector<uint8_t>& output) {
    if (plaintext.empty() || passphrase.empty()) {
        return false;
    }
    
    std::vector<uint8_t> salt(SALT_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);
    std::vector<uint8_t> key(KEY_SIZE);
    
    // Generate cryptographically secure random salt and IV
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1 ||
        RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Derive encryption key from passphrase
    if (!deriveKey(passphrase, salt, key)) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    CipherContext ctx;
    
    // Initialize AES-256-GCM encryption
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Set key and IV
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Prepare output buffer
    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int outlen = 0;
    int total_len = 0;
    
    // Encrypt plaintext
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &outlen,
                          reinterpret_cast<const uint8_t*>(plaintext.data()),
                          static_cast<int>(plaintext.size())) != 1) {
        secureClear(key.data(), key.size());
        secureClear(ciphertext.data(), ciphertext.size());
        return false;
    }
    total_len = outlen;
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + outlen, &outlen) != 1) {
        secureClear(key.data(), key.size());
        secureClear(ciphertext.data(), ciphertext.size());
        return false;
    }
    total_len += outlen;
    ciphertext.resize(total_len);
    
    // Get authentication tag
    std::vector<uint8_t> tag(TAG_SIZE);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
        secureClear(key.data(), key.size());
        secureClear(ciphertext.data(), ciphertext.size());
        return false;
    }
    
    // Build output: [magic][version][salt][iv][ciphertext][tag]
    output.clear();
    output.insert(output.end(), FILE_MAGIC, FILE_MAGIC + 4);
    output.push_back(FILE_VERSION);
    output.insert(output.end(), salt.begin(), salt.end());
    output.insert(output.end(), iv.begin(), iv.end());
    output.insert(output.end(), ciphertext.begin(), ciphertext.end());
    output.insert(output.end(), tag.begin(), tag.end());
    
    // Securely clear sensitive data
    secureClear(key.data(), key.size());
    secureClear(ciphertext.data(), ciphertext.size());
    
    return true;
}

// Decrypt sensitive data using AES-256-GCM
bool decryptData(const std::vector<uint8_t>& input, const std::string& passphrase,
                 std::string& plaintext) {
    if (passphrase.empty()) {
        return false;
    }
    
    // Validate minimum size: magic(4) + version(1) + salt(16) + iv(12) + tag(16) = 49 bytes
    if (input.size() < 49) {
        return false;
    }
    
    // Validate magic and version
    if (std::memcmp(input.data(), FILE_MAGIC, 4) != 0 || input[4] != FILE_VERSION) {
        return false;
    }
    
    // Extract components
    std::vector<uint8_t> salt(input.begin() + 5, input.begin() + 5 + SALT_SIZE);
    std::vector<uint8_t> iv(input.begin() + 5 + SALT_SIZE, input.begin() + 5 + SALT_SIZE + IV_SIZE);
    size_t ciphertext_start = 5 + SALT_SIZE + IV_SIZE;
    size_t ciphertext_len = input.size() - ciphertext_start - TAG_SIZE;
    std::vector<uint8_t> ciphertext(input.begin() + ciphertext_start, 
                                     input.begin() + ciphertext_start + ciphertext_len);
    std::vector<uint8_t> tag(input.end() - TAG_SIZE, input.end());
    
    // Derive key
    std::vector<uint8_t> key(KEY_SIZE);
    if (!deriveKey(passphrase, salt, key)) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    CipherContext ctx;
    
    // Initialize AES-256-GCM decryption
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Set key and IV
    if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) {
        secureClear(key.data(), key.size());
        return false;
    }
    
    // Prepare output buffer
    std::vector<uint8_t> plaintext_buf(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int outlen = 0;
    int total_len = 0;
    
    // Decrypt ciphertext
    if (EVP_DecryptUpdate(ctx.get(), plaintext_buf.data(), &outlen,
                          ciphertext.data(), static_cast<int>(ciphertext.size())) != 1) {
        secureClear(key.data(), key.size());
        secureClear(plaintext_buf.data(), plaintext_buf.size());
        return false;
    }
    total_len = outlen;
    
    // Set expected authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()) != 1) {
        secureClear(key.data(), key.size());
        secureClear(plaintext_buf.data(), plaintext_buf.size());
        return false;
    }
    
    // Finalize decryption and verify tag
    int ret = EVP_DecryptFinal_ex(ctx.get(), plaintext_buf.data() + outlen, &outlen);
    
    if (ret != 1) {
        // Tag verification failed - do not reveal partial plaintext
        secureClear(key.data(), key.size());
        secureClear(plaintext_buf.data(), plaintext_buf.size());
        return false;
    }
    total_len += outlen;
    
    plaintext.assign(reinterpret_cast<char*>(plaintext_buf.data()), total_len);
    
    // Securely clear sensitive data
    secureClear(key.data(), key.size());
    secureClear(plaintext_buf.data(), plaintext_buf.size());
    
    return true;
}

// Save encrypted data to file with atomic write
bool saveToFile(const std::string& filename, const std::vector<uint8_t>& data) {
    if (filename.empty() || data.empty()) {
        return false;
    }
    
    // Write to temporary file first for atomic operation
    std::string temp_filename = filename + ".tmp";
    std::ofstream file(temp_filename, std::ios::binary | std::ios::trunc);
    
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.flush();
    
    if (!file.good()) {
        file.close();
        std::remove(temp_filename.c_str());
        return false;
    }
    
    file.close();
    
    // Atomic rename
    if (std::rename(temp_filename.c_str(), filename.c_str()) != 0) {
        std::remove(temp_filename.c_str());
        return false;
    }
    
    return true;
}

// Load encrypted data from file
bool loadFromFile(const std::string& filename, std::vector<uint8_t>& data) {
    if (filename.empty()) {
        return false;
    }
    
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    
    std::streamsize size = file.tellg();
    if (size <= 0 || size > 10 * 1024 * 1024) { // Max 10MB
        return false;
    }
    
    file.seekg(0, std::ios::beg);
    data.resize(static_cast<size_t>(size));
    
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        data.clear();
        return false;
    }
    
    return true;
}

int main() {
    // Test case 1: Encrypt and decrypt credit card number
    {
        std::string sensitive_data = "4532-1234-5678-9010";
        std::string passphrase = "StrongPassphrase123!@#";
        std::vector<uint8_t> encrypted;
        
        if (encryptData(sensitive_data, passphrase, encrypted)) {
            std::string decrypted;
            if (decryptData(encrypted, passphrase, decrypted)) {
                std::cout << "Test 1 PASSED: Credit card encryption/decryption successful" << std::endl;
            } else {
                std::cout << "Test 1 FAILED: Decryption failed" << std::endl;
            }
        } else {
            std::cout << "Test 1 FAILED: Encryption failed" << std::endl;
        }
    }
    
    // Test case 2: Encrypt personal information and save to file
    {
        std::string personal_info = "SSN:123-45-6789|DOB:1990-01-01|Name:John Doe";
        std::string passphrase = "SecurePassword456$%^";
        std::vector<uint8_t> encrypted;
        
        if (encryptData(personal_info, passphrase, encrypted)) {
            if (saveToFile("personal_data.enc", encrypted)) {
                std::vector<uint8_t> loaded;
                if (loadFromFile("personal_data.enc", loaded)) {
                    std::string decrypted;
                    if (decryptData(loaded, passphrase, decrypted)) {
                        std::cout << "Test 2 PASSED: File save/load and decryption successful" << std::endl;
                    } else {
                        std::cout << "Test 2 FAILED: Decryption failed" << std::endl;
                    }
                } else {
                    std::cout << "Test 2 FAILED: File load failed" << std::endl;
                }
                std::remove("personal_data.enc");
            } else {
                std::cout << "Test 2 FAILED: File save failed" << std::endl;
            }
        } else {
            std::cout << "Test 2 FAILED: Encryption failed" << std::endl;
        }
    }
    
    // Test case 3: Wrong passphrase should fail
    {
        std::string data = "Secret Information";
        std::string correct_pass = "CorrectPassword789";
        std::string wrong_pass = "WrongPassword789";
        std::vector<uint8_t> encrypted;
        
        if (encryptData(data, correct_pass, encrypted)) {
            std::string decrypted;
            if (!decryptData(encrypted, wrong_pass, decrypted)) {
                std::cout << "Test 3 PASSED: Wrong passphrase correctly rejected" << std::endl;
            } else {
                std::cout << "Test 3 FAILED: Wrong passphrase accepted" << std::endl;
            }
        } else {
            std::cout << "Test 3 FAILED: Encryption failed" << std::endl;
        }
    }
    
    // Test case 4: Empty data handling
    {
        std::string empty_data = "";
        std::string passphrase = "TestPassword";
        std::vector<uint8_t> encrypted;
        
        if (!encryptData(empty_data, passphrase, encrypted)) {
            std::cout << "Test 4 PASSED: Empty data correctly rejected" << std::endl;
        } else {
            std::cout << "Test 4 FAILED: Empty data accepted" << std::endl;
        }
    }
    
    // Test case 5: Multiple encryptions with same passphrase produce different ciphertexts
    {
        std::string data = "Test Data";
        std::string passphrase = "SamePassphrase";
        std::vector<uint8_t> encrypted1, encrypted2;
        
        if (encryptData(data, passphrase, encrypted1) && 
            encryptData(data, passphrase, encrypted2)) {
            if (encrypted1 != encrypted2) {
                std::cout << "Test 5 PASSED: Unique IVs generate different ciphertexts" << std::endl;
            } else {
                std::cout << "Test 5 FAILED: Same ciphertext produced" << std::endl;
            }
        } else {
            std::cout << "Test 5 FAILED: Encryption failed" << std::endl;
        }
    }
    
    return 0;
}
