// To compile and run this code:
// 1. Install OpenSSL development libraries (e.g., on Debian/Ubuntu: sudo apt-get install libssl-dev)
// 2. Compile with: g++ this_file.cpp -o task83_cpp -lssl -lcrypto
// 3. Run: ./task83_cpp

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h> // For AES_BLOCK_SIZE

// Custom exception for OpenSSL errors
class OpenSSLException : public std::runtime_error {
public:
    OpenSSLException(const std::string& msg) : std::runtime_error(msg) {
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        details_ = "OpenSSL Error: " + std::string(err_buf);
    }
    const char* what() const noexcept override {
        return details_.c_str();
    }
private:
    std::string details_;
};

std::vector<unsigned char> encrypt(const std::string& plainText, const unsigned char* key, const unsigned char* iv) {
    EVP_CIPHER_CTX *ctx = nullptr;
    std::vector<unsigned char> ciphertext;
    int len = 0;
    int ciphertext_len = 0;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        throw OpenSSLException("Failed to create new EVP_CIPHER_CTX");
    }

    // Initialize the encryption operation.
    // We use AES-128-CBC, so key and IV must be 16 bytes.
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        throw OpenSSLException("Failed to initialize encryption");
    }

    // Allocate buffer for ciphertext. It can be larger than plaintext due to padding.
    // The max size is plaintext_len + block_size.
    ciphertext.resize(plainText.length() + AES_BLOCK_SIZE);

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                               reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length())) {
        EVP_CIPHER_CTX_free(ctx);
        throw OpenSSLException("Failed in EVP_EncryptUpdate");
    }
    ciphertext_len = len;

    // Finalize the encryption.
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw OpenSSLException("Failed in EVP_EncryptFinal_ex");
    }
    ciphertext_len += len;
    
    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// Helper function to print byte vector as hex
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl; // Reset to decimal mode
}

int main() {
    // A 128-bit key and IV (16 bytes)
    const unsigned char key[] = "0123456789abcdef";
    const unsigned char iv[] = "fedcba9876543210";

    const std::vector<std::string> testCases = {
        "This is a test.",
        "Short",
        "This is a slightly longer plaintext message.",
        "Encrypt this key!",
        "AES CBC Mode Test"
    };
    
    std::cout << "AES-128-CBC Encryption Test Cases:" << std::endl;
    std::cout << "Key: " << key << std::endl;
    std::cout << "IV: " << iv << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

    for (size_t i = 0; i < testCases.size(); ++i) {
        try {
            std::cout << "Test Case " << (i + 1) << ":" << std::endl;
            std::cout << "  Plaintext:  " << testCases[i] << std::endl;
            std::vector<unsigned char> encrypted = encrypt(testCases[i], key, iv);
            print_hex("  Encrypted (Hex): ", encrypted);
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Encryption failed for test case " << (i + 1) << ": " << e.what() << std::endl;
        }
    }

    return 0;
}