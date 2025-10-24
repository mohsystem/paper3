#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

// Helper to convert binary data to a Base64 string for printing
std::string to_base64(const std::vector<unsigned char>& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    std::string base64_str(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    return base64_str;
}

// Function to generate a 2048-bit RSA key pair using OpenSSL's EVP interface
EVP_PKEY* generateRsaKeyPair() {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new_id failed.");

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_keygen_init failed.");
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_keygen_bits failed.");
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_keygen failed.");
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

// Encrypts data using the RSA public key with OAEP padding
std::vector<unsigned char> encrypt(const std::string& plainText, EVP_PKEY* pkey) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new failed.");

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_encrypt_init failed.");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_padding failed.");
    }

    size_t out_len;
    if (EVP_PKEY_encrypt(ctx, nullptr, &out_len, (const unsigned char*)plainText.c_str(), plainText.length()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_encrypt (size check) failed.");
    }

    std::vector<unsigned char> encrypted(out_len);
    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &out_len, (const unsigned char*)plainText.c_str(), plainText.length()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_encrypt failed.");
    }

    encrypted.resize(out_len);
    EVP_PKEY_CTX_free(ctx);
    return encrypted;
}

// Decrypts data using the RSA private key with OAEP padding
std::string decrypt(const std::vector<unsigned char>& cipherText, EVP_PKEY* pkey) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new failed.");
    
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_decrypt_init failed.");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_padding failed.");
    }

    size_t out_len;
    if (EVP_PKEY_decrypt(ctx, nullptr, &out_len, cipherText.data(), cipherText.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_decrypt (size check) failed.");
    }

    std::vector<unsigned char> decrypted(out_len);
    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &out_len, cipherText.data(), cipherText.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_decrypt failed.");
    }

    decrypted.resize(out_len);
    EVP_PKEY_CTX_free(ctx);
    return std::string(decrypted.begin(), decrypted.end());
}


int main() {
    // Note: To compile, you need to link against OpenSSL, e.g., g++ your_file.cpp -o your_app -lssl -lcrypto
    const std::vector<std::string> testCases = {
        "This is a secret message.",
        "RSA is a public-key cryptosystem.",
        "Security is important.",
        "Test case 4 with numbers 12345.",
        "Final test case with symbols !@#$%"
    };

    std::cout << "--- C++ RSA Encryption/Decryption ---" << std::endl;

    for (int i = 0; i < testCases.size(); ++i) {
        const std::string& originalMessage = testCases[i];
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Original Message: " << originalMessage << std::endl;

        EVP_PKEY* pkey = nullptr;
        try {
            // 1. Generate Key Pair
            pkey = generateRsaKeyPair();

            // 2. Encrypt the message
            std::vector<unsigned char> encryptedMessage = encrypt(originalMessage, pkey);
            std::string encodedMessage = to_base64(encryptedMessage);
            std::cout << "Encrypted (Base64): " << encodedMessage << std::endl;

            // 3. Decrypt the message
            std::string decryptedMessage = decrypt(encryptedMessage, pkey);
            std::cout << "Decrypted Message: " << decryptedMessage << std::endl;

            if (originalMessage != decryptedMessage) {
                std::cerr << "Error: Decrypted message does not match original." << std::endl;
            }

        } catch (const std::runtime_error& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;
            char err_buf[256];
            ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
            std::cerr << "OpenSSL Error: " << err_buf << std::endl;
        }

        // Clean up the key
        if (pkey) {
            EVP_PKEY_free(pkey);
        }
    }

    return 0;
}