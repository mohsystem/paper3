#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Note: To compile this code, you need to link against OpenSSL libraries.
// Example: g++ your_file.cpp -o your_program -lssl -lcrypto

// Helper function to convert a byte vector to a hex string for printing
std::string to_hex(const std::vector<unsigned char>& data) {
    std::string hex_string;
    const char hex_chars[] = "0123456789abcdef";
    for (unsigned char c : data) {
        hex_string += hex_chars[c >> 4];
        hex_string += hex_chars[c & 0x0f];
    }
    return hex_string;
}

/**
 * @brief Encrypts a message using AES-256-GCM.
 * 
 * @param plainText The message to encrypt.
 * @param key The 32-byte (256-bit) secret key.
 * @return A byte vector containing [IV][Ciphertext][Tag].
 */
std::vector<unsigned char> encrypt(const std::string& plainText, const std::vector<unsigned char>& key) {
    if (key.size() != 32) { // AES-256 key size
        throw std::runtime_error("Invalid key size. Key must be 32 bytes for AES-256.");
    }

    const int IV_LENGTH = 12; // Recommended for GCM
    const int TAG_LENGTH = 16; // Standard tag size

    // 1. Generate a random IV
    std::vector<unsigned char> iv(IV_LENGTH);
    if (!RAND_bytes(iv.data(), iv.size())) {
        throw std::runtime_error("Failed to generate random IV.");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX.");
    }

    // 2. Create and initialize the cipher context
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption with key and IV.");
    }

    std::vector<unsigned char> ciphertext(plainText.length());
    int len;

    // 3. Encrypt the message
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                              reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed.");
    }
    int ciphertext_len = len;

    // Finalize encryption (GCM does not typically produce more data here)
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed.");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    std::vector<unsigned char> tag(TAG_LENGTH);
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get GCM tag.");
    }

    EVP_CIPHER_CTX_free(ctx);

    // Combine IV, ciphertext, and tag into a single message
    std::vector<unsigned char> encrypted_message;
    encrypted_message.reserve(iv.size() + ciphertext.size() + tag.size());
    encrypted_message.insert(encrypted_message.end(), iv.begin(), iv.end());
    encrypted_message.insert(encrypted_message.end(), ciphertext.begin(), ciphertext.end());
    encrypted_message.insert(encrypted_message.end(), tag.begin(), tag.end());

    return encrypted_message;
}

int main() {
    // In a real application, this key should be managed securely.
    std::vector<unsigned char> key(32); // 256-bit key
    if (!RAND_bytes(key.data(), key.size())) {
        std::cerr << "Failed to generate random key." << std::endl;
        return 1;
    }
    std::cout << "Using Secret Key (Hex): " << to_hex(key) << std::endl;
    std::cout << "---" << std::endl;

    std::vector<std::string> testCases = {
        "This is a secret message.",
        "Hello, World!",
        "Cryptography is fascinating.",
        "AES-GCM provides authenticated encryption.",
        "12345!@#$%^"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        try {
            std::cout << "Test Case " << i + 1 << std::endl;
            std::cout << "Original:  " << testCases[i] << std::endl;
            std::vector<unsigned char> encrypted = encrypt(testCases[i], key);
            std::cout << "Encrypted (Hex): " << to_hex(encrypted) << std::endl << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl << std::endl;
        }
    }

    return 0;
}