#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <memory>
#include <iomanip>

// RAII wrappers for OpenSSL objects
struct EVP_PKEY_CTX_deleter {
    void operator()(EVP_PKEY_CTX* ctx) const { if (ctx) EVP_PKEY_CTX_free(ctx); }
};
struct EVP_PKEY_deleter {
    void operator()(EVP_PKEY* pkey) const { if (pkey) EVP_PKEY_free(pkey); }
};
using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_deleter>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, EVP_PKEY_deleter>;


void handle_openssl_error() {
    throw std::runtime_error("OpenSSL error: " + std::to_string(ERR_get_error()));
}

// Generates a 2048-bit RSA key pair
EVP_PKEY* generate_rsa_key_pair() {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
    if (!ctx) handle_openssl_error();

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) handle_openssl_error();
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) <= 0) handle_openssl_error();

    if (EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) handle_openssl_error();

    return pkey;
}

std::vector<unsigned char> encrypt(const std::string& plain_text, EVP_PKEY* public_key) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(public_key, nullptr));
    if (!ctx) handle_openssl_error();

    if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) handle_openssl_error();
    
    // Rule #8: Always incorporate OAEP when using the RSA algorithm
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) handle_openssl_error();
    
    size_t out_len = 0;
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &out_len, (const unsigned char*)plain_text.c_str(), plain_text.length()) <= 0) handle_openssl_error();
    
    std::vector<unsigned char> cipher_text(out_len);
    if (EVP_PKEY_encrypt(ctx.get(), cipher_text.data(), &out_len, (const unsigned char*)plain_text.c_str(), plain_text.length()) <= 0) handle_openssl_error();
    
    cipher_text.resize(out_len);
    return cipher_text;
}

std::string decrypt(const std::vector<unsigned char>& cipher_text, EVP_PKEY* private_key) {
    EVP_PKEY_CTX_ptr ctx(EVP_PKEY_CTX_new(private_key, nullptr));
    if (!ctx) handle_openssl_error();

    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) handle_openssl_error();
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) handle_openssl_error();

    size_t out_len = 0;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &out_len, cipher_text.data(), cipher_text.size()) <= 0) handle_openssl_error();
    
    std::vector<unsigned char> plain_text_bytes(out_len);
    if (EVP_PKEY_decrypt(ctx.get(), plain_text_bytes.data(), &out_len, cipher_text.data(), cipher_text.size()) <= 0) handle_openssl_error();
    
    plain_text_bytes.resize(out_len);
    return std::string(plain_text_bytes.begin(), plain_text_bytes.end());
}

void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        std::cout << std::setw(2) << static_cast<int>(c);
    }
    std::cout << std::dec << std::endl;
}

int main() {
    EVP_PKEY_ptr key_pair(generate_rsa_key_pair());
    if (!key_pair) {
        std::cerr << "Failed to generate key pair." << std::endl;
        return 1;
    }

    std::vector<std::string> test_cases = {
        "This is a test message.",
        "RSA with OAEP padding is secure.",
        "Another message for testing purposes.",
        "Short",
        "A much longer message to see how the encryption handles different lengths of input data."
    };

    int i = 1;
    for (const auto& original_message : test_cases) {
        std::cout << "--- Test Case " << i++ << " ---" << std::endl;
        std::cout << "Original: " << original_message << std::endl;

        try {
            std::vector<unsigned char> encrypted = encrypt(original_message, key_pair.get());
            print_hex("Encrypted (Hex): ", encrypted);

            std::string decrypted = decrypt(encrypted, key_pair.get());
            std::cout << "Decrypted: " << decrypted << std::endl;

            if (original_message == decrypted) {
                std::cout << "Status: SUCCESS" << std::endl;
            } else {
                std::cout << "Status: FAILED" << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
            std::cout << "Status: FAILED" << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}