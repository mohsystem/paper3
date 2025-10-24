#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/err.h>

// To compile: g++ your_file.cpp -o your_app -lssl -lcrypto
// Make sure you have OpenSSL development libraries installed (e.g., libssl-dev on Debian/Ubuntu).

// Static Initialization Vector (IV). For AES, this is 16 bytes (128 bits).
// In a real-world application, IV should be random and unique for each encryption.
const unsigned char STATIC_IV[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
}

/**
 * Encrypts data using AES-128-CBC with a static IV.
 * PKCS padding is enabled by default in EVP_EncryptInit_ex.
 *
 * @param key The 16-byte (128-bit) encryption key.
 * @param plaintext The data to encrypt.
 * @return A vector containing the encrypted data. Returns an empty vector on failure.
 */
std::vector<unsigned char> encrypt(const unsigned char* key, const std::string& plaintext) {
    std::vector<unsigned char> ciphertext;
    EVP_CIPHER_CTX* ctx = nullptr;
    int len = 0;
    int ciphertext_len = 0;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
        return {};
    }

    // Initialize the encryption operation.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    // The output buffer needs to be large enough for the encrypted data plus the final block.
    // plaintext.length() + block_size is a safe allocation size.
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    ciphertext.resize(plaintext.length() + block_size);

    // Provide the message to be encrypted, and obtain the encrypted output.
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                              reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length())) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len = len;

    // Finalize the encryption. This will handle the padding.
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

int main() {
    // A 16-byte (128-bit) key for demonstration.
    // In a real application, use a secure key management system.
    const unsigned char key[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };

    std::vector<std::string> test_cases = {
        "Hello, World!",
        "16-byte message!", // Exactly one block
        "This is a longer test message for AES CBC encryption.",
        "", // Empty string
        "123!@#$%^&*()"
    };

    std::cout << "C++ AES/CBC Encryption with Static IV" << std::endl;
    std::cout << "------------------------------------" << std::endl;

    for (int i = 0; i < test_cases.size(); ++i) {
        std::cout << "Test Case #" << (i + 1) << std::endl;
        std::cout << "Plaintext: " << test_cases[i] << std::endl;
        std::vector<unsigned char> encrypted_data = encrypt(key, test_cases[i]);
        if (!encrypted_data.empty()) {
            print_hex("Encrypted (Hex): ", encrypted_data);
        } else {
            std::cout << "Encryption failed." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}