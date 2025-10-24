
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>

// Secure encryption using AES-256-GCM with unique IV per operation
// Rejects insecure static IV and CBC mode per security rules
class SecureEncryption {
private:
    static constexpr size_t SALT_SIZE = 16;
    static constexpr size_t IV_SIZE = 12;
    static constexpr size_t TAG_SIZE = 16;
    static constexpr size_t KEY_SIZE = 32;
    static constexpr int PBKDF2_ITERATIONS = 210000;

    // Derive key from passphrase using PBKDF2-HMAC-SHA256
    static std::vector<unsigned char> deriveKey(const std::string& passphrase, 
                                                 const std::vector<unsigned char>& salt) {
        if (passphrase.empty()) {
            throw std::invalid_argument("Passphrase cannot be empty");
        }
        if (salt.size() != SALT_SIZE) {
            throw std::invalid_argument("Invalid salt size");
        }

        std::vector<unsigned char> key(KEY_SIZE);
        
        // Use PBKDF2 for key derivation per Rules#5 and #6
        if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), passphrase.length(),
                              salt.data(), salt.size(),
                              PBKDF2_ITERATIONS,
                              EVP_sha256(),
                              KEY_SIZE, key.data()) != 1) {
            throw std::runtime_error("Key derivation failed");
        }
        
        return key;
    }

    // Generate cryptographically secure random bytes
    static std::vector<unsigned char> generateRandom(size_t size) {
        std::vector<unsigned char> buffer(size);
        
        // Use OpenSSL CSPRNG per Rules#5 and #6
        if (RAND_bytes(buffer.data(), size) != 1) {
            throw std::runtime_error("Random generation failed");
        }
        
        return buffer;
    }

public:
    // Encrypt data using AES-256-GCM with unique IV
    // Returns: [salt(16)][iv(12)][ciphertext][tag(16)]
    static std::vector<unsigned char> encrypt(const std::string& plaintext,
                                              const std::string& passphrase) {
        // Validate inputs per Rules#1
        if (plaintext.empty()) {
            throw std::invalid_argument("Plaintext cannot be empty");
        }
        if (passphrase.empty()) {
            throw std::invalid_argument("Passphrase cannot be empty");
        }

        // Generate unique salt and IV per encryption per Rules#5
        std::vector<unsigned char> salt = generateRandom(SALT_SIZE);
        std::vector<unsigned char> iv = generateRandom(IV_SIZE);
        
        // Derive encryption key
        std::vector<unsigned char> key = deriveKey(passphrase, salt);

        // Initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Initialize AES-256-GCM encryption per Rules#5 and #6
            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                throw std::runtime_error("Cipher initialization failed");
            }

            // Set IV length
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
                throw std::runtime_error("IV length setting failed");
            }

            // Set key and IV
            if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) {
                throw std::runtime_error("Key/IV setting failed");
            }

            // Prepare output buffer
            std::vector<unsigned char> ciphertext(plaintext.length() + EVP_CIPHER_CTX_block_size(ctx));
            int len = 0;
            int ciphertext_len = 0;

            // Encrypt plaintext
            if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                                 reinterpret_cast<const unsigned char*>(plaintext.data()),
                                 plaintext.length()) != 1) {
                throw std::runtime_error("Encryption failed");
            }
            ciphertext_len = len;

            // Finalize encryption
            if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
                throw std::runtime_error("Encryption finalization failed");
            }
            ciphertext_len += len;
            ciphertext.resize(ciphertext_len);

            // Get authentication tag per Rules#5
            std::vector<unsigned char> tag(TAG_SIZE);
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
                throw std::runtime_error("Tag generation failed");
            }

            // Clear sensitive key material per Rules#1
            OPENSSL_cleanse(key.data(), key.size());

            // Build output: [salt][iv][ciphertext][tag]
            std::vector<unsigned char> result;
            result.reserve(SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE);
            result.insert(result.end(), salt.begin(), salt.end());
            result.insert(result.end(), iv.begin(), iv.end());
            result.insert(result.end(), ciphertext.begin(), ciphertext.end());
            result.insert(result.end(), tag.begin(), tag.end());

            EVP_CIPHER_CTX_free(ctx);
            return result;

        } catch (...) {
            OPENSSL_cleanse(key.data(), key.size());
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }
    }

    // Decrypt data encrypted with AES-256-GCM
    static std::string decrypt(const std::vector<unsigned char>& encrypted,
                               const std::string& passphrase) {
        // Validate input size per Rules#1
        if (encrypted.size() < SALT_SIZE + IV_SIZE + TAG_SIZE) {
            throw std::invalid_argument("Invalid encrypted data size");
        }
        if (passphrase.empty()) {
            throw std::invalid_argument("Passphrase cannot be empty");
        }

        // Extract components
        std::vector<unsigned char> salt(encrypted.begin(), encrypted.begin() + SALT_SIZE);
        std::vector<unsigned char> iv(encrypted.begin() + SALT_SIZE, 
                                     encrypted.begin() + SALT_SIZE + IV_SIZE);
        size_t ciphertext_start = SALT_SIZE + IV_SIZE;
        size_t ciphertext_len = encrypted.size() - SALT_SIZE - IV_SIZE - TAG_SIZE;
        std::vector<unsigned char> tag(encrypted.end() - TAG_SIZE, encrypted.end());

        // Derive decryption key
        std::vector<unsigned char> key = deriveKey(passphrase, salt);

        // Initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        try {
            // Initialize AES-256-GCM decryption
            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
                throw std::runtime_error("Cipher initialization failed");
            }

            // Set IV length
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
                throw std::runtime_error("IV length setting failed");
            }

            // Set key and IV
            if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) {
                throw std::runtime_error("Key/IV setting failed");
            }

            // Prepare output buffer
            std::vector<unsigned char> plaintext(ciphertext_len);
            int len = 0;
            int plaintext_len = 0;

            // Decrypt ciphertext
            if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                                 encrypted.data() + ciphertext_start,
                                 ciphertext_len) != 1) {
                throw std::runtime_error("Decryption failed");
            }
            plaintext_len = len;

            // Set expected tag per Rules#5
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE,
                                   const_cast<unsigned char*>(tag.data())) != 1) {
                throw std::runtime_error("Tag setting failed");
            }

            // Finalize and verify tag - fail closed on mismatch per Rules#5
            int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
            
            // Clear sensitive key material per Rules#1
            OPENSSL_cleanse(key.data(), key.size());
            
            if (ret != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Authentication failed");
            }
            
            plaintext_len += len;
            plaintext.resize(plaintext_len);

            EVP_CIPHER_CTX_free(ctx);
            return std::string(plaintext.begin(), plaintext.end());

        } catch (...) {
            OPENSSL_cleanse(key.data(), key.size());
            EVP_CIPHER_CTX_free(ctx);
            throw;
        }
    }
};

int main() {
    try {
        // Test case 1: Basic encryption/decryption
        std::string plaintext1 = "Hello, World!";
        std::string passphrase1 = "SecurePassphrase123!";
        auto encrypted1 = SecureEncryption::encrypt(plaintext1, passphrase1);
        std::string decrypted1 = SecureEncryption::decrypt(encrypted1, passphrase1);
        std::cout << "Test 1: " << (decrypted1 == plaintext1 ? "PASS" : "FAIL") << std::endl;

        // Test case 2: Longer text
        std::string plaintext2 = "This is a longer message to test encryption with multiple blocks.";
        std::string passphrase2 = "AnotherSecureKey456$";
        auto encrypted2 = SecureEncryption::encrypt(plaintext2, passphrase2);
        std::string decrypted2 = SecureEncryption::decrypt(encrypted2, passphrase2);
        std::cout << "Test 2: " << (decrypted2 == plaintext2 ? "PASS" : "FAIL") << std::endl;

        // Test case 3: Special characters
        std::string plaintext3 = "Special chars: !@#$%^&*(){}[]|\\\\:;\\"'<>,.?/~`";
        std::string passphrase3 = "ComplexPass789&*";
        auto encrypted3 = SecureEncryption::encrypt(plaintext3, passphrase3);
        std::string decrypted3 = SecureEncryption::decrypt(encrypted3, passphrase3);
        std::cout << "Test 3: " << (decrypted3 == plaintext3 ? "PASS" : "FAIL") << std::endl;

        // Test case 4: Verify unique IVs produce different ciphertexts
        auto encrypted4a = SecureEncryption::encrypt(plaintext1, passphrase1);
        auto encrypted4b = SecureEncryption::encrypt(plaintext1, passphrase1);
        std::cout << "Test 4: " << (encrypted4a != encrypted4b ? "PASS" : "FAIL") << std::endl;

        // Test case 5: Wrong passphrase should fail
        try {
            SecureEncryption::decrypt(encrypted1, "WrongPassphrase");
            std::cout << "Test 5: FAIL (should have thrown)" << std::endl;
        } catch (const std::exception&) {
            std::cout << "Test 5: PASS (correctly rejected wrong passphrase)" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
