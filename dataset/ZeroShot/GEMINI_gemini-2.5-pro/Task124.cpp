/*
 * DISCLAIMER: This is a demonstration of cryptographic concepts.
 * It is NOT a secure system for storing sensitive data in a production environment.
 * Real-world security requires a much more comprehensive approach, including secure
 * key management, which is not possible in this single-file example.
 *
 * COMPILE: g++ -Wall -std=c++17 this_file.cpp -o task_cpp -lssl -lcrypto
 * You MUST have OpenSSL development libraries installed (e.g., `libssl-dev` on Debian/Ubuntu).
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/kdf.h>

// --- Constants ---
const int AES_KEY_SIZE = 256;
const int GCM_IV_LENGTH = 12; // 96 bits
const int GCM_TAG_LENGTH = 16; // 128 bits
const int SALT_LENGTH = 16;
const int PBKDF2_ITERATIONS = 600000; // Match Python example

// Custom unique_ptr deleters for OpenSSL types
struct BIO_deleter { void operator()(BIO* p) const { BIO_free_all(p); } };
using unique_BIO = std::unique_ptr<BIO, BIO_deleter>;

// Helper for Base64 encoding
std::string base64_encode(const std::vector<unsigned char>& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_push(b64, mem);

    unique_BIO bio(mem);

    if (BIO_write(bio.get(), data.data(), data.size()) <= 0) {
        throw std::runtime_error("Failed to base64 encode");
    }
    BIO_flush(bio.get());

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio.get(), &bptr);

    std::string result(bptr->data, bptr->length);
    return result;
}

// Helper for Base64 decoding
std::vector<unsigned char> base64_decode(const std::string& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* mem = BIO_new_mem_buf(data.c_str(), data.length());
    mem = BIO_push(b64, mem);
    
    unique_BIO bio(mem);

    std::vector<unsigned char> decoded(data.length()); // Max possible size
    int decoded_len = BIO_read(bio.get(), decoded.data(), decoded.size());

    if (decoded_len <= 0) {
        throw std::runtime_error("Failed to base64 decode");
    }
    decoded.resize(decoded_len);
    return decoded;
}

std::string encrypt(const std::string& plainText, const std::string& password) {
    // 1. Generate salt
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }

    // 2. Derive key
    std::vector<unsigned char> key(AES_KEY_SIZE / 8);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITERATIONS, EVP_sha256(), key.size(), key.data()) != 1) {
        throw std::runtime_error("Failed to derive key");
    }

    // 3. Generate IV
    std::vector<unsigned char> iv(GCM_IV_LENGTH);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }

    // 4. Encrypt
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    std::vector<unsigned char> ciphertext(plainText.length());
    std::vector<unsigned char> tag(GCM_TAG_LENGTH);
    int len, ciphertext_len;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data());
    
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length());
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data());
    EVP_CIPHER_CTX_free(ctx);

    // 5. Format output
    return base64_encode(salt) + ":" + base64_encode(iv) + ":" + base64_encode(tag) + ":" + base64_encode(ciphertext);
}

std::string decrypt(const std::string& encryptedData, const std::string& password) {
    // 1. Parse input
    std::string s_salt_b64, s_iv_b64, s_tag_b64, s_ciphertext_b64;
    size_t pos1 = encryptedData.find(':');
    size_t pos2 = encryptedData.find(':', pos1 + 1);
    size_t pos3 = encryptedData.find(':', pos2 + 1);

    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
        throw std::runtime_error("Invalid encrypted data format");
    }
    s_salt_b64 = encryptedData.substr(0, pos1);
    s_iv_b64 = encryptedData.substr(pos1 + 1, pos2 - pos1 - 1);
    s_tag_b64 = encryptedData.substr(pos2 + 1, pos3 - pos2 - 1);
    s_ciphertext_b64 = encryptedData.substr(pos3 + 1);
    
    std::vector<unsigned char> salt = base64_decode(s_salt_b64);
    std::vector<unsigned char> iv = base64_decode(s_iv_b64);
    std::vector<unsigned char> tag = base64_decode(s_tag_b64);
    std::vector<unsigned char> ciphertext = base64_decode(s_ciphertext_b64);

    // 2. Re-derive key
    std::vector<unsigned char> key(AES_KEY_SIZE / 8);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITERATIONS, EVP_sha256(), key.size(), key.data()) != 1) {
        throw std::runtime_error("Failed to derive key");
    }

    // 3. Decrypt
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    std::vector<unsigned char> plaintext(ciphertext.size());
    int len, plaintext_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data());
    
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
    plaintext_len = len;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data());

    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    if (ret <= 0) {
        throw std::runtime_error("Decryption failed: Invalid password or corrupted data");
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);

    return std::string(plaintext.begin(), plaintext.end());
}

int main() {
    std::cout << "Running C++ Tests...\n";
    std::string password = "a-very-strong-password-!@#$";

    std::vector<std::string> testData = {
        "1234-5678-9012-3456", // Test Case 1: Credit Card
        "{\"name\":\"John Doe\", \"ssn\":\"000-00-0000\"}", // Test Case 2: JSON data
        "My secret diary entry.", // Test Case 3: Simple text
        "", // Test Case 4: Empty string
        "Data with special characters: <>?`|~" // Test Case 5: Special characters
    };

    for (int i = 0; i < testData.size(); ++i) {
        const std::string& originalData = testData[i];
        std::cout << "\n--- Test Case " << (i + 1) << " ---\n";
        std::cout << "Original:    " << originalData << std::endl;
        try {
            std::string encrypted = encrypt(originalData, password);
            std::cout << "Encrypted:   " << encrypted << std::endl;

            std::string decrypted = decrypt(encrypted, password);
            std::cout << "Decrypted:   " << decrypted << std::endl;

            if (originalData != decrypted) {
                std::cout << "FAILURE: Decrypted data does not match original data." << std::endl;
            } else {
                std::cout << "SUCCESS: Data integrity verified." << std::endl;
            }

            std::cout << "Testing with incorrect password..." << std::endl;
            try {
                decrypt(encrypted, "wrong-password");
                std::cout << "FAILURE: Decryption with wrong password should have failed but didn't." << std::endl;
            } catch (const std::runtime_error& e) {
                std::cout << "SUCCESS: Decryption failed as expected with wrong password. (" << e.what() << ")" << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "An error occurred during test case " << (i + 1) << ": " << e.what() << std::endl;
        }
    }

    return 0;
}