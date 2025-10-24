
#include <iostream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <cstring>

class Task79 {
private:
    static const int KEY_LENGTH = 32; // 256 bits
    static const int ITERATION_COUNT = 65536;
    static const int SALT_LENGTH = 16;
    static const int IV_LENGTH = 16;

    static std::string base64Encode(const unsigned char* buffer, size_t length) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);

        return result;
    }

public:
    static std::string encryptMessage(const std::string& message, const std::string& secretKey) {
        try {
            // Generate random salt
            unsigned char salt[SALT_LENGTH];
            if (RAND_bytes(salt, SALT_LENGTH) != 1) {
                throw std::runtime_error("Failed to generate salt");
            }

            // Derive key from password using PBKDF2
            unsigned char key[KEY_LENGTH];
            if (PKCS5_PBKDF2_HMAC(secretKey.c_str(), secretKey.length(),
                                  salt, SALT_LENGTH,
                                  ITERATION_COUNT,
                                  EVP_sha256(),
                                  KEY_LENGTH, key) != 1) {
                throw std::runtime_error("Key derivation failed");
            }

            // Generate random IV
            unsigned char iv[IV_LENGTH];
            if (RAND_bytes(iv, IV_LENGTH) != 1) {
                throw std::runtime_error("Failed to generate IV");
            }

            // Create cipher context
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                throw std::runtime_error("Failed to create cipher context");
            }

            // Initialize encryption
            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Encryption initialization failed");
            }

            // Encrypt message
            std::vector<unsigned char> ciphertext(message.length() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
            int len = 0, ciphertext_len = 0;

            if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                                  reinterpret_cast<const unsigned char*>(message.c_str()),
                                  message.length()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Encryption update failed");
            }
            ciphertext_len = len;

            if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Encryption finalization failed");
            }
            ciphertext_len += len;

            EVP_CIPHER_CTX_free(ctx);

            // Combine salt + IV + encrypted data
            std::vector<unsigned char> combined(SALT_LENGTH + IV_LENGTH + ciphertext_len);
            std::memcpy(combined.data(), salt, SALT_LENGTH);
            std::memcpy(combined.data() + SALT_LENGTH, iv, IV_LENGTH);
            std::memcpy(combined.data() + SALT_LENGTH + IV_LENGTH, ciphertext.data(), ciphertext_len);

            // Encode to Base64
            return base64Encode(combined.data(), combined.size());
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Encryption failed: ") + e.what());
        }
    }
};

int main() {
    std::vector<std::string> messages = {
        "Hello, World!",
        "Secure message 123",
        "This is a confidential document",
        "Test@#$%^&*()",
        "Another secret message"
    };
    
    std::vector<std::string> keys = {
        "MySecretKey123",
        "P@ssw0rd!Strong",
        "EncryptionKey456",
        "SecurePassword789",
        "TopSecret2024"
    };

    std::cout << "=== Encryption Test Cases ===\\n\\n";
    for (size_t i = 0; i < messages.size(); i++) {
        std::cout << "Test Case " << (i + 1) << ":\\n";
        std::cout << "Original Message: " << messages[i] << "\\n";
        std::cout << "Secret Key: " << keys[i] << "\\n";
        try {
            std::string encrypted = Task79::encryptMessage(messages[i], keys[i]);
            std::cout << "Encrypted Message: " << encrypted << "\\n";
            std::cout << "Encrypted Length: " << encrypted.length() << " characters\\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\\n";
        }
        std::cout << "\\n";
    }

    return 0;
}
