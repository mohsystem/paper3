#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Note: This code requires the OpenSSL library (version 1.1.1 or newer recommended).
// To compile on Linux/macOS: g++ your_file.cpp -o rsa_test -lssl -lcrypto
// You may need to install development packages like libssl-dev or openssl-devel.

// --- Custom Deleters for OpenSSL unique_ptr for RAII ---
struct EVP_PKEY_CTX_deleter {
    void operator()(EVP_PKEY_CTX* ctx) const { if (ctx) EVP_PKEY_CTX_free(ctx); }
};

struct EVP_PKEY_deleter {
    void operator()(EVP_PKEY* pkey) const { if (pkey) EVP_PKEY_free(pkey); }
};

using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_deleter>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, EVP_PKEY_deleter>;

// --- Function Declarations ---
EVP_PKEY_ptr generate_keys() {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL));
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new_id failed.");

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) throw std::runtime_error("EVP_PKEY_keygen_init failed.");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) <= 0) throw std::runtime_error("EVP_PKEY_CTX_set_rsa_keygen_bits failed.");
    
    EVP_PKEY *pkey = NULL;
    if (EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) throw std::runtime_error("EVP_PKEY_keygen failed.");

    return EVP_PKEY_ptr(pkey);
}

std::vector<unsigned char> rsa_encrypt(const std::string& data, EVP_PKEY* pkey) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(pkey, NULL));
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new for encryption failed.");

    if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) throw std::runtime_error("EVP_PKEY_encrypt_init failed.");

    // Use OAEP padding for security, which is the recommended standard.
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) throw std::runtime_error("EVP_PKEY_CTX_set_rsa_padding failed.");

    size_t out_len;
    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx.get(), NULL, &out_len, reinterpret_cast<const unsigned char*>(data.c_str()), data.length()) <= 0) {
        throw std::runtime_error("EVP_PKEY_encrypt length check failed.");
    }
    
    std::vector<unsigned char> encrypted(out_len);
    if (EVP_PKEY_encrypt(ctx.get(), encrypted.data(), &out_len, reinterpret_cast<const unsigned char*>(data.c_str()), data.length()) <= 0) {
        throw std::runtime_error("EVP_PKEY_encrypt failed.");
    }
    
    encrypted.resize(out_len); // Resize to actual encrypted size
    return encrypted;
}

std::string rsa_decrypt(const std::vector<unsigned char>& encrypted_data, EVP_PKEY* pkey) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(pkey, NULL));
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new for decryption failed.");

    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) throw std::runtime_error("EVP_PKEY_decrypt_init failed.");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) throw std::runtime_error("EVP_PKEY_CTX_set_rsa_padding failed.");

    size_t out_len;
    // Determine buffer length
    if (EVP_PKEY_decrypt(ctx.get(), NULL, &out_len, encrypted_data.data(), encrypted_data.size()) <= 0) {
        throw std::runtime_error("EVP_PKEY_decrypt length check failed.");
    }

    std::vector<unsigned char> decrypted(out_len);
    if (EVP_PKEY_decrypt(ctx.get(), decrypted.data(), &out_len, encrypted_data.data(), encrypted_data.size()) <= 0) {
        throw std::runtime_error("EVP_PKEY_decrypt failed.");
    }

    return std::string(reinterpret_cast<char*>(decrypted.data()), out_len);
}

// Helper to print bytes in hex for readability
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    for (unsigned char c : data) {
        printf("%02x", c);
    }
    std::cout << std::endl;
}

int main() {
    try {
        EVP_PKEY_ptr pkey = generate_keys();
        if (!pkey) {
            std::cerr << "Key generation failed." << std::endl;
            return 1;
        }

        const char* test_cases[] = {
            "This is a test message.",
            "RSA is a public-key cryptosystem.",
            "Security is important.",
            "12345!@#$%^&*()",
            "A short message."
        };

        for (int i = 0; i < 5; ++i) {
            std::string original_data(test_cases[i]);
            std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
            std::cout << "Original: " << original_data << std::endl;

            std::vector<unsigned char> encrypted_data = rsa_encrypt(original_data, pkey.get());
            print_hex("Encrypted (hex): ", encrypted_data);
            
            std::string decrypted_data = rsa_decrypt(encrypted_data, pkey.get());
            std::cout << "Decrypted: " << decrypted_data << std::endl;
            
            std::cout << "Status: " << (original_data == decrypted_data ? "SUCCESS" : "FAILURE") << std::endl;
            std::cout << std::endl;
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        // Print any OpenSSL-specific errors
        ERR_print_errors_fp(stderr);
        return 1;
    }

    return 0;
}