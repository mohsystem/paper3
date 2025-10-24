
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <stdexcept>

// File format: [magic(4)][version(1)][salt(16)][iv(12)][ciphertext][tag(16)]
constexpr size_t SALT_SIZE = 16;
constexpr size_t IV_SIZE = 12;
constexpr size_t TAG_SIZE = 16;
constexpr size_t KEY_SIZE = 32; // AES-256
constexpr int PBKDF2_ITERATIONS = 210000;
const char MAGIC[] = "ENC1";
constexpr uint8_t VERSION = 1;

class SecureEncryptor {
private:
    // Securely clear memory before deallocation
    static void secure_clear(void* ptr, size_t size) {
        if (ptr != nullptr && size > 0) {
            volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
            for (size_t i = 0; i < size; ++i) {
                p[i] = 0;
            }
        }
    }

    // Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256
    static bool derive_key(const std::string& passphrase, 
                          const unsigned char* salt,
                          unsigned char* key) {
        if (passphrase.empty() || salt == nullptr || key == nullptr) {
            return false;
        }

        // Use PBKDF2-HMAC-SHA-256 with 210,000 iterations for key derivation
        int result = PKCS5_PBKDF2_HMAC(
            passphrase.c_str(),
            static_cast<int>(passphrase.length()),
            salt,
            SALT_SIZE,
            PBKDF2_ITERATIONS,
            EVP_sha256(),
            KEY_SIZE,
            key
        );

        return result == 1;
    }

public:
    // Encrypt plaintext using AES-256-GCM
    static std::vector<unsigned char> encrypt(const std::string& plaintext,
                                              const std::string& passphrase) {
        // Validate inputs - treat all inputs as untrusted
        if (plaintext.empty()) {
            throw std::invalid_argument("Plaintext cannot be empty");
        }
        if (passphrase.empty()) {
            throw std::invalid_argument("Passphrase cannot be empty");
        }
        if (plaintext.length() > 1073741824) { // 1GB limit
            throw std::invalid_argument("Plaintext too large");
        }

        unsigned char salt[SALT_SIZE];
        unsigned char iv[IV_SIZE];
        unsigned char key[KEY_SIZE];
        unsigned char tag[TAG_SIZE];

        // Initialize all sensitive buffers to zero
        std::memset(salt, 0, SALT_SIZE);
        std::memset(iv, 0, IV_SIZE);
        std::memset(key, 0, KEY_SIZE);
        std::memset(tag, 0, TAG_SIZE);

        try {
            // Generate cryptographically secure random salt (unique per encryption)
            if (RAND_bytes(salt, SALT_SIZE) != 1) {
                throw std::runtime_error("Failed to generate salt");
            }

            // Generate cryptographically secure random IV (unique per encryption, never reuse)
            if (RAND_bytes(iv, IV_SIZE) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Failed to generate IV");
            }

            // Derive encryption key from passphrase using PBKDF2
            if (!derive_key(passphrase, salt, key)) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Key derivation failed");
            }

            // Create cipher context (RAII-style cleanup)
            std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> 
                ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
            
            if (!ctx) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Failed to create cipher context");
            }

            // Initialize AES-256-GCM encryption
            if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Encryption initialization failed");
            }

            // Set IV length (12 bytes for GCM)
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Failed to set IV length");
            }

            // Set key and IV
            if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key, iv) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Failed to set key and IV");
            }

            // Allocate output buffer with bounds checking
            size_t ciphertext_len = plaintext.length() + EVP_CIPHER_CTX_block_size(ctx.get());
            if (ciphertext_len < plaintext.length()) { // Check for integer overflow
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Integer overflow in buffer size calculation");
            }

            std::vector<unsigned char> ciphertext(ciphertext_len);
            int len = 0;

            // Perform encryption
            if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len,
                                 reinterpret_cast<const unsigned char*>(plaintext.data()),
                                 static_cast<int>(plaintext.length())) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Encryption failed");
            }

            int ciphertext_total_len = len;

            // Finalize encryption
            if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                throw std::runtime_error("Encryption finalization failed");
            }

            ciphertext_total_len += len;

            // Get the 16-byte authentication tag
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Failed to get authentication tag");
            }

            // Build output: [magic][version][salt][iv][ciphertext][tag]
            std::vector<unsigned char> output;
            output.reserve(4 + 1 + SALT_SIZE + IV_SIZE + ciphertext_total_len + TAG_SIZE);

            // Add magic bytes
            output.insert(output.end(), MAGIC, MAGIC + 4);
            
            // Add version
            output.push_back(VERSION);
            
            // Add salt
            output.insert(output.end(), salt, salt + SALT_SIZE);
            
            // Add IV
            output.insert(output.end(), iv, iv + IV_SIZE);
            
            // Add ciphertext
            output.insert(output.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_total_len);
            
            // Add authentication tag
            output.insert(output.end(), tag, tag + TAG_SIZE);

            // Securely clear all sensitive data
            secure_clear(salt, SALT_SIZE);
            secure_clear(iv, IV_SIZE);
            secure_clear(key, KEY_SIZE);
            secure_clear(tag, TAG_SIZE);

            return output;

        } catch (...) {
            // Ensure cleanup on any exception
            secure_clear(salt, SALT_SIZE);
            secure_clear(iv, IV_SIZE);
            secure_clear(key, KEY_SIZE);
            secure_clear(tag, TAG_SIZE);
            throw;
        }
    }

    // Decrypt ciphertext using AES-256-GCM
    static std::string decrypt(const std::vector<unsigned char>& encrypted_data,
                               const std::string& passphrase) {
        // Validate inputs
        if (passphrase.empty()) {
            throw std::invalid_argument("Passphrase cannot be empty");
        }

        // Validate minimum size: magic(4) + version(1) + salt(16) + iv(12) + tag(16) = 49
        const size_t MIN_SIZE = 4 + 1 + SALT_SIZE + IV_SIZE + TAG_SIZE;
        if (encrypted_data.size() < MIN_SIZE) {
            throw std::invalid_argument("Invalid encrypted data: too short");
        }

        size_t pos = 0;

        // Validate magic bytes
        if (std::memcmp(encrypted_data.data(), MAGIC, 4) != 0) {
            throw std::invalid_argument("Invalid file format: magic bytes mismatch");
        }
        pos += 4;

        // Validate version
        if (encrypted_data[pos] != VERSION) {
            throw std::invalid_argument("Unsupported version");
        }
        pos += 1;

        // Extract salt
        unsigned char salt[SALT_SIZE];
        std::memcpy(salt, encrypted_data.data() + pos, SALT_SIZE);
        pos += SALT_SIZE;

        // Extract IV
        unsigned char iv[IV_SIZE];
        std::memcpy(iv, encrypted_data.data() + pos, IV_SIZE);
        pos += IV_SIZE;

        // Calculate ciphertext length
        size_t ciphertext_len = encrypted_data.size() - pos - TAG_SIZE;
        if (ciphertext_len > 1073741824) { // 1GB limit
            secure_clear(salt, SALT_SIZE);
            secure_clear(iv, IV_SIZE);
            throw std::invalid_argument("Ciphertext too large");
        }

        // Extract tag (last 16 bytes)
        unsigned char tag[TAG_SIZE];
        std::memcpy(tag, encrypted_data.data() + encrypted_data.size() - TAG_SIZE, TAG_SIZE);

        unsigned char key[KEY_SIZE];
        std::memset(key, 0, KEY_SIZE);

        try {
            // Derive key from passphrase
            if (!derive_key(passphrase, salt, key)) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Key derivation failed");
            }

            // Create cipher context
            std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> 
                ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
            
            if (!ctx) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Failed to create cipher context");
            }

            // Initialize AES-256-GCM decryption
            if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Decryption initialization failed");
            }

            // Set IV length
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Failed to set IV length");
            }

            // Set key and IV
            if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key, iv) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Failed to set key and IV");
            }

            // Allocate plaintext buffer
            std::vector<unsigned char> plaintext(ciphertext_len + EVP_CIPHER_CTX_block_size(ctx.get()));
            int len = 0;

            // Decrypt
            if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len,
                                 encrypted_data.data() + pos,
                                 static_cast<int>(ciphertext_len)) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Decryption failed");
            }

            int plaintext_len = len;

            // Set expected tag value for verification
            if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                throw std::runtime_error("Failed to set authentication tag");
            }

            // Finalize and verify tag - fail closed on verification failure
            int ret = EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len);
            if (ret <= 0) {
                secure_clear(salt, SALT_SIZE);
                secure_clear(iv, IV_SIZE);
                secure_clear(key, KEY_SIZE);
                secure_clear(tag, TAG_SIZE);
                secure_clear(plaintext.data(), plaintext.size());
                throw std::runtime_error("Authentication failed: invalid tag or corrupted data");
            }

            plaintext_len += len;

            // Convert to string
            std::string result(plaintext.begin(), plaintext.begin() + plaintext_len);

            // Securely clear all sensitive data
            secure_clear(salt, SALT_SIZE);
            secure_clear(iv, IV_SIZE);
            secure_clear(key, KEY_SIZE);
            secure_clear(tag, TAG_SIZE);
            secure_clear(plaintext.data(), plaintext.size());

            return result;

        } catch (...) {
            secure_clear(salt, SALT_SIZE);
            secure_clear(iv, IV_SIZE);
            secure_clear(key, KEY_SIZE);
            secure_clear(tag, TAG_SIZE);
            throw;
        }
    }
};

int main() {
    try {
        // Test case 1: Basic encryption and decryption
        {
            std::string plaintext = "Hello, World!";
            std::string passphrase = "SecurePassword123!";
            
            auto encrypted = SecureEncryptor::encrypt(plaintext, passphrase);
            std::string decrypted = SecureEncryptor::decrypt(encrypted, passphrase);
            
            std::cout << "Test 1: " << (plaintext == decrypted ? "PASS" : "FAIL") << std::endl;
        }

        // Test case 2: Longer text
        {
            std::string plaintext = "This is a longer text to test the encryption algorithm with multiple blocks.";
            std::string passphrase = "AnotherSecurePass456@";
            
            auto encrypted = SecureEncryptor::encrypt(plaintext, passphrase);
            std::string decrypted = SecureEncryptor::decrypt(encrypted, passphrase);
            
            std::cout << "Test 2: " << (plaintext == decrypted ? "PASS" : "FAIL") << std::endl;
        }

        // Test case 3: Special characters
        {
            std::string plaintext = "Special chars: !@#$%^&*()_+-=[]{}|;:',.<>?/~`";
            std::string passphrase = "ComplexPass789#";
            
            auto encrypted = SecureEncryptor::encrypt(plaintext, passphrase);
            std::string decrypted = SecureEncryptor::decrypt(encrypted, passphrase);
            
            std::cout << "Test 3: " << (plaintext == decrypted ? "PASS" : "FAIL") << std::endl;
        }

        // Test case 4: Unicode characters
        {
            std::string plaintext = "Unicode test: こんにちは世界 🌍🔒";
            std::string passphrase = "UnicodeTest2024!";
            
            auto encrypted = SecureEncryptor::encrypt(plaintext, passphrase);
            std::string decrypted = SecureEncryptor::decrypt(encrypted, passphrase);
            
            std::cout << "Test 4: " << (plaintext == decrypted ? "PASS" : "FAIL") << std::endl;
        }

        // Test case 5: Wrong passphrase (should fail)
        {
            std::string plaintext = "Secret message";
            std::string passphrase1 = "CorrectPassword!";
            std::string passphrase2 = "WrongPassword!";
            
            auto encrypted = SecureEncryptor::encrypt(plaintext, passphrase1);
            
            bool failed_as_expected = false;
            try {
                std::string decrypted = SecureEncryptor::decrypt(encrypted, passphrase2);
            } catch (const std::exception&) {
                failed_as_expected = true;
            }
            
            std::cout << "Test 5: " << (failed_as_expected ? "PASS" : "FAIL") << " (wrong password detection)" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
