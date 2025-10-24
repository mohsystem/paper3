
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdexcept>

class Task52 {
private:
    static const int KEY_SIZE = 32; // 256 bits
    static const int IV_LENGTH = 12; // GCM nonce
    static const int TAG_LENGTH = 16; // GCM tag

public:
    static std::vector<unsigned char> generateKey() {
        std::vector<unsigned char> key(KEY_SIZE);
        if (RAND_bytes(key.data(), KEY_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }
        return key;
    }

    static std::vector<unsigned char> encryptFile(
        const std::vector<unsigned char>& fileContent,
        const std::vector<unsigned char>& key) {
        
        if (fileContent.empty() || key.size() != KEY_SIZE) {
            throw std::invalid_argument("Invalid input parameters");
        }

        // Generate random IV
        std::vector<unsigned char> iv(IV_LENGTH);
        if (RAND_bytes(iv.data(), IV_LENGTH) != 1) {
            throw std::runtime_error("Failed to generate IV");
        }

        // Create and initialize context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        // Initialize encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        // Encrypt data
        std::vector<unsigned char> ciphertext(fileContent.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len = 0;
        int ciphertext_len = 0;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, fileContent.data(), fileContent.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);

        // Get tag
        std::vector<unsigned char> tag(TAG_LENGTH);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to get tag");
        }

        EVP_CIPHER_CTX_free(ctx);

        // Combine IV + ciphertext + tag
        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        result.insert(result.end(), tag.begin(), tag.end());

        return result;
    }

    static std::vector<unsigned char> decryptFile(
        const std::vector<unsigned char>& encryptedContent,
        const std::vector<unsigned char>& key) {
        
        if (encryptedContent.size() < IV_LENGTH + TAG_LENGTH || key.size() != KEY_SIZE) {
            throw std::invalid_argument("Invalid encrypted content or key");
        }

        // Extract IV, ciphertext, and tag
        std::vector<unsigned char> iv(encryptedContent.begin(), encryptedContent.begin() + IV_LENGTH);
        std::vector<unsigned char> tag(encryptedContent.end() - TAG_LENGTH, encryptedContent.end());
        std::vector<unsigned char> ciphertext(
            encryptedContent.begin() + IV_LENGTH,
            encryptedContent.end() - TAG_LENGTH
        );

        // Create and initialize context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        // Decrypt data
        std::vector<unsigned char> plaintext(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len = 0;
        int plaintext_len = 0;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption failed");
        }
        plaintext_len = len;

        // Set tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LENGTH, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set tag");
        }

        // Finalize decryption
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption finalization failed - authentication failed");
        }
        plaintext_len += len;
        plaintext.resize(plaintext_len);

        EVP_CIPHER_CTX_free(ctx);

        return plaintext;
    }
};

int main() {
    std::cout << "=== Secure File Encryption/Decryption Test Cases ===" << std::endl << std::endl;

    try {
        // Test Case 1: Simple text encryption/decryption
        std::cout << "Test Case 1: Simple text encryption" << std::endl;
        auto key1 = Task52::generateKey();
        std::string text1 = "Hello, World!";
        std::vector<unsigned char> content1(text1.begin(), text1.end());
        auto encrypted1 = Task52::encryptFile(content1, key1);
        auto decrypted1 = Task52::decryptFile(encrypted1, key1);
        std::string result1(decrypted1.begin(), decrypted1.end());
        std::cout << "Original: " << text1 << std::endl;
        std::cout << "Decrypted: " << result1 << std::endl;
        std::cout << "Match: " << (text1 == result1 ? "true" : "false") << std::endl << std::endl;

        // Test Case 2: Empty content
        std::cout << "Test Case 2: Empty content" << std::endl;
        auto key2 = Task52::generateKey();
        std::vector<unsigned char> content2;
        auto encrypted2 = Task52::encryptFile(content2, key2);
        auto decrypted2 = Task52::decryptFile(encrypted2, key2);
        std::cout << "Original length: " << content2.size() << std::endl;
        std::cout << "Decrypted length: " << decrypted2.size() << std::endl;
        std::cout << "Match: " << (content2.size() == decrypted2.size() ? "true" : "false") << std::endl << std::endl;

        // Test Case 3: Large text
        std::cout << "Test Case 3: Large text encryption" << std::endl;
        auto key3 = Task52::generateKey();
        std::string text3;
        for (int i = 0; i < 1000; i++) {
            text3 += "Secure encryption test " + std::to_string(i) + " ";
        }
        std::vector<unsigned char> content3(text3.begin(), text3.end());
        auto encrypted3 = Task52::encryptFile(content3, key3);
        auto decrypted3 = Task52::decryptFile(encrypted3, key3);
        std::cout << "Original length: " << content3.size() << std::endl;
        std::cout << "Decrypted length: " << decrypted3.size() << std::endl;
        std::cout << "Match: " << (content3 == decrypted3 ? "true" : "false") << std::endl << std::endl;

        // Test Case 4: Special characters
        std::cout << "Test Case 4: Special characters" << std::endl;
        auto key4 = Task52::generateKey();
        std::string text4 = "Special chars: !@#$%^&*()_+-=[]{}|;':\\",./<>?";
        std::vector<unsigned char> content4(text4.begin(), text4.end());
        auto encrypted4 = Task52::encryptFile(content4, key4);
        auto decrypted4 = Task52::decryptFile(encrypted4, key4);
        std::string result4(decrypted4.begin(), decrypted4.end());
        std::cout << "Original: " << text4 << std::endl;
        std::cout << "Decrypted: " << result4 << std::endl;
        std::cout << "Match: " << (text4 == result4 ? "true" : "false") << std::endl << std::endl;

        // Test Case 5: Binary data
        std::cout << "Test Case 5: Binary data" << std::endl;
        auto key5 = Task52::generateKey();
        std::vector<unsigned char> content5;
        for (int i = 0; i < 256; i++) {
            content5.push_back(static_cast<unsigned char>(i));
        }
        auto encrypted5 = Task52::encryptFile(content5, key5);
        auto decrypted5 = Task52::decryptFile(encrypted5, key5);
        std::cout << "Original length: " << content5.size() << std::endl;
        std::cout << "Decrypted length: " << decrypted5.size() << std::endl;
        std::cout << "Match: " << (content5 == decrypted5 ? "true" : "false") << std::endl << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
