// Note: This code requires the OpenSSL library.
// You need to link against it, e.g., g++ your_file.cpp -o your_app -lcrypto
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/err.h>

// 1. Create a static initialization vector (IV).
const unsigned char STATIC_IV[] = "abcdefghijklmnop";
// A secret key is also required for AES. Using a 128-bit key (16 bytes).
const unsigned char SECRET_KEY[] = "0123456789abcdef";

// Helper to print vector of bytes as a hex string
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for(const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

std::vector<unsigned char> encrypt(const std::string& plainText, const unsigned char* key, const unsigned char* iv) {
    EVP_CIPHER_CTX *ctx = nullptr;
    int len = 0;
    int ciphertext_len = 0;
    std::vector<unsigned char> ciphertext;

    try {
        if(!(ctx = EVP_CIPHER_CTX_new())) {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        // 2. Initialize a cipher using AES in CBC mode with the static IV.
        // Using AES-128 in CBC mode.
        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        }

        // 3. Pad the input data to match the block size required by the cipher.
        // OpenSSL's EVP functions handle PKCS#7 padding by default.
        // The output buffer needs to be large enough for the plaintext plus one block for padding.
        ciphertext.resize(plainText.length() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));

        // 4. Encrypt the padded data using the cipher.
        if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                                  reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length())) {
            throw std::runtime_error("EVP_EncryptUpdate failed");
        }
        ciphertext_len = len;

        if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        ciphertext.resize(ciphertext_len);

        // 5. Return the encrypted data.
        return ciphertext;

    } catch (...) {
        if (ctx) EVP_CIPHER_CTX_free(ctx);
        throw; // re-throw the exception
    }
}

class Task72 {
public:
    static void main() {
        std::vector<std::string> testCases = {
            "",
            "Hello",
            "16-byte message!", // Exactly 16 bytes, will add a full padding block
            "This is a longer test message for AES encryption.",
            "!@#$%^&*()_+"
        };

        std::cout << "CPP AES/CBC Encryption Test Cases:" << std::endl;
        for (size_t i = 0; i < testCases.size(); ++i) {
            try {
                std::vector<unsigned char> encrypted = encrypt(testCases[i], SECRET_KEY, STATIC_IV);
                std::cout << "Test Case " << (i + 1) << ":" << std::endl;
                std::cout << "  Plaintext:  \"" << testCases[i] << "\"" << std::endl;
                print_hex("  Encrypted (Hex):    ", encrypted);
                std::cout << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error during encryption for test case " << (i + 1) << ": " << e.what() << std::endl;
            }
        }
    }
};

int main() {
    Task72::main();
    return 0;
}