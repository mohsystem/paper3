#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Note: To compile, you need to link against OpenSSL libraries.
// Example: g++ your_source_file.cpp -o your_program -lssl -lcrypto

// Helper to print OpenSSL errors
void handleOpenSSLErrors() {
    char err_buf[256];
    ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
    throw std::runtime_error("OpenSSL Error: " + std::string(err_buf));
}

// Helper to print a vector of bytes as a hex string
void printHex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

// Helper to Base64 encode data
std::string base64Encode(const unsigned char* data, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string encoded(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return encoded;
}

/**
 * Encrypts plaintext using AES-256-CBC.
 *
 * @param plainText The data to encrypt.
 * @param key The 256-bit (32-byte) encryption key.
 * @param iv The 128-bit (16-byte) initialization vector.
 * @return The ciphertext as a vector of unsigned chars.
 */
std::vector<unsigned char> encrypt(const std::string& plainText, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {
    if (key.size() != 32) throw std::invalid_argument("Key must be 32 bytes for AES-256.");
    if (iv.size() != 16) throw std::invalid_argument("IV must be 16 bytes for AES.");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleOpenSSLErrors();

    std::vector<unsigned char> ciphertext(plainText.length() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int ciphertext_len = 0;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        handleOpenSSLErrors();
    }

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length())) {
        EVP_CIPHER_CTX_free(ctx);
        handleOpenSSLErrors();
    }
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        handleOpenSSLErrors();
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

class Task83 {
public:
    static void run() {
        // Generate a single, reusable key for the test cases
        std::vector<unsigned char> key(32);
        if (!RAND_bytes(key.data(), key.size())) {
            handleOpenSSLErrors();
        }
        printHex("Generated AES Key: ", key);
        std::cout << "----------------------------------------" << std::endl;

        std::vector<std::string> testCases = {
            "This is a secret message.",
            "The quick brown fox jumps over the lazy dog.",
            "AES encryption is secure!",
            "1234567890",
            "Another test case with some special characters !@#$%^&*()"
        };
        
        for (int i = 0; i < testCases.size(); ++i) {
            std::cout << "Test Case " << (i + 1) << std::endl;
            std::cout << "Plaintext: " << testCases[i] << std::endl;

            // Generate a new, random IV for each encryption
            std::vector<unsigned char> iv(16);
            if (!RAND_bytes(iv.data(), iv.size())) {
                handleOpenSSLErrors();
            }
            printHex("IV: ", iv);

            try {
                std::vector<unsigned char> ciphertext = encrypt(testCases[i], key, iv);
                
                // Prepend IV to ciphertext
                std::vector<unsigned char> encrypted_data = iv;
                encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());

                // Base64 encode for printing
                std::string encoded_result = base64Encode(encrypted_data.data(), encrypted_data.size());

                std::cout << "Encrypted (Base64): " << encoded_result << std::endl;

            } catch (const std::exception& e) {
                std::cerr << "Encryption failed: " << e.what() << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;
        }
    }
};

int main() {
    Task83::run();
    return 0;
}