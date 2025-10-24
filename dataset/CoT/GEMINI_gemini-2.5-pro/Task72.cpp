#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/err.h>

// IMPORTANT: Using a static, predictable IV is a major security vulnerability
// in modes like CBC. The IV should be random and unique for each encryption
// operation with the same key. This code is for demonstration purposes only,
// following the prompt's specific requirements.
static const unsigned char STATIC_IV[] = "1234567890123456";

// Helper to print OpenSSL error messages
void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
}

/**
 * Encrypts data using AES in CBC mode with a static IV.
 * @param plainText The data to encrypt.
 * @param key The secret key for encryption (must be 16 bytes for AES-128).
 * @return A vector of unsigned chars containing the encrypted data. Returns an empty vector on failure.
 */
std::vector<unsigned char> encrypt(const std::string& plainText, const unsigned char* key) {
    std::vector<unsigned char> cipherText(plainText.length() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int ciphertext_len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handle_openssl_errors();
        return {};
    }

    // Initialize the cipher using AES in CBC mode with the static IV.
    // OpenSSL's EVP_* encrypt functions handle PKCS#7 padding by default.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    // Encrypt the padded data.
    if (1 != EVP_EncryptUpdate(ctx, cipherText.data(), &len, 
                              reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length())) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len = len;

    // Finalize the encryption (handles final block and padding).
    if (1 != EVP_EncryptFinal_ex(ctx, cipherText.data() + len, &len)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    cipherText.resize(ciphertext_len);
    return cipherText;
}

// Helper to print byte vector as a hex string
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

class Task72 {
public:
    static void run_tests() {
        // IMPORTANT: Hardcoding keys is a security risk. In a real application,
        // use a secure key management system.
        // 16 bytes for AES-128
        const unsigned char key[] = "thisisakeyjklmno";

        std::vector<std::string> testCases = {
            "This is test case 1.",
            "Another test case with different data.",
            "Short",
            "A much longer string to test padding and multiple blocks of encryption.",
            "Test case 5 is the final one!"
        };

        std::cout << "Encrypting using AES/CBC with a STATIC IV (INSECURE PRACTICE)" << std::endl;
        std::cout << "==========================================================" << std::endl;

        for (size_t i = 0; i < testCases.size(); ++i) {
            std::cout << "Test Case " << (i + 1) << ":" << std::endl;
            std::cout << "  Plaintext:  " << testCases[i] << std::endl;

            std::vector<unsigned char> encrypted = encrypt(testCases[i], key);
            
            if (!encrypted.empty()) {
                print_hex("  Encrypted (Hex): ", encrypted);
            } else {
                std::cout << "  Encryption failed." << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    // To compile: g++ your_file_name.cpp -o your_program_name -lssl -lcrypto
    Task72::run_tests();
    return 0;
}