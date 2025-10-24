/*
 * Compile with: g++ Task83.cpp -o Task83 -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

const int AES_KEY_SIZE_BYTES = 32; // 256 bits
const int AES_IV_SIZE_BYTES = 16;  // 128 bits for AES block size

// Helper function to print a byte vector as a hex string
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << ss.str() << std::endl;
}

// Helper function to handle and report OpenSSL errors
void handle_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        std::cerr << "OpenSSL Error: " << err_buf << std::endl;
    }
    throw std::runtime_error("An OpenSSL error occurred.");
}

/**
 * Creates a Cipher using AES-256-CBC, encrypts the given plaintext,
 * and returns the IV prepended to the ciphertext.
 *
 * @param plainText The string to encrypt.
 * @param key       The 256-bit (32-byte) key for encryption.
 * @return A vector of unsigned char containing the IV followed by the ciphertext.
 */
std::vector<unsigned char> encrypt(const std::string& plainText, const std::vector<unsigned char>& key) {
    if (key.size() != AES_KEY_SIZE_BYTES) {
        throw std::invalid_argument("Key must be 32 bytes (256 bits) long.");
    }

    // 1. Generate a cryptographically strong random IV
    std::vector<unsigned char> iv(AES_IV_SIZE_BYTES);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        handle_openssl_errors();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handle_openssl_errors();
    }

    // 2. Initialize the encryption operation with AES-256-CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }

    // 3. Encrypt the data
    // Ciphertext can be up to one block larger than plaintext due to padding
    std::vector<unsigned char> cipherText(plainText.length() + AES_IV_SIZE_BYTES);
    int len = 0;
    if (EVP_EncryptUpdate(ctx, cipherText.data(), &len, 
                          reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    int ciphertext_len = len;

    // Finalize the encryption (this handles padding)
    if (EVP_EncryptFinal_ex(ctx, cipherText.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext_len += len;
    
    cipherText.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);

    // 4. Prepend the IV to the ciphertext
    std::vector<unsigned char> encryptedData;
    encryptedData.reserve(iv.size() + cipherText.size());
    encryptedData.insert(encryptedData.end(), iv.begin(), iv.end());
    encryptedData.insert(encryptedData.end(), cipherText.begin(), cipherText.end());

    return encryptedData;
}

int main() {
    ERR_load_crypto_strings();
    
    try {
        // Generate a secure AES key
        std::vector<unsigned char> secret_key(AES_KEY_SIZE_BYTES);
        if (RAND_bytes(secret_key.data(), secret_key.size()) != 1) {
            handle_openssl_errors();
        }

        print_hex("Using AES Key (Hex): ", secret_key);
        std::cout << "--- Running 5 Test Cases ---" << std::endl;

        std::vector<std::string> testCases = {
            "This is the first test case.",
            "Encrypting another piece of data.",
            "AES in CBC mode is secure with a random IV.",
            "A short string.",
            "A much longer string to test padding and block chaining properly, ensuring everything works as expected for various lengths of input data."
        };
        
        for (size_t i = 0; i < testCases.size(); ++i) {
            const std::string& plainText = testCases[i];
            std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
            std::cout << "Plaintext: " << plainText << std::endl;
            
            std::vector<unsigned char> encryptedData = encrypt(plainText, secret_key);
            
            print_hex("Encrypted (Hex): ", encryptedData);
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }
    
    ERR_free_strings();
    EVP_cleanup(); // Deprecated in OpenSSL 1.1.0+, but harmless
    
    return 0;
}