#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Note: This code requires linking with OpenSSL libraries.
// Example compilation command: g++ your_file.cpp -o your_program -lssl -lcrypto

// AES-GCM parameters
const int AES_KEY_SIZE_BYTES = 32; // 256 bits
const int GCM_IV_LENGTH = 12;      // 96 bits
const int GCM_TAG_LENGTH = 16;     // 128 bits

/**
 * Base64 encodes a byte vector.
 */
std::string base64_encode(const std::vector<unsigned char>& input) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.data(), input.size());
    BIO_flush(bio);

    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    std::string encoded_data(buffer_ptr->data, buffer_ptr->length);

    BIO_free_all(bio);
    return encoded_data;
}

/**
 * Generates a cryptographically secure key.
 * In a real application, you must manage keys securely.
 */
std::vector<unsigned char> generate_key() {
    std::vector<unsigned char> key(AES_KEY_SIZE_BYTES);
    if (RAND_bytes(key.data(), key.size()) != 1) {
        throw std::runtime_error("Failed to generate random key");
    }
    return key;
}

/**
 * Encrypts a plaintext string using AES-256-GCM.
 * The output is a Base64 encoded string containing the IV, ciphertext, and tag.
 *
 * @param plaintext The string to encrypt.
 * @param key The 32-byte secret key.
 * @return A Base64 encoded string representing the encrypted data.
 */
std::string encrypt(const std::string& plaintext, const std::vector<unsigned char>& key) {
    if (key.size() != AES_KEY_SIZE_BYTES) {
        throw std::invalid_argument("Key must be 32 bytes.");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    std::vector<unsigned char> iv(GCM_IV_LENGTH);
    if (RAND_bytes(iv.data(), GCM_IV_LENGTH) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to generate IV");
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize GCM encryption");
    }

    std::vector<unsigned char> ciphertext(plaintext.length());
    int len;
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                               reinterpret_cast<const unsigned char*>(plaintext.c_str()), 
                               plaintext.length())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    int ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    std::vector<unsigned char> tag(GCM_TAG_LENGTH);
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get GCM tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    // Combine IV, ciphertext, and tag for transport
    std::vector<unsigned char> cipher_message;
    cipher_message.reserve(GCM_IV_LENGTH + ciphertext.size() + GCM_TAG_LENGTH);
    cipher_message.insert(cipher_message.end(), iv.begin(), iv.end());
    cipher_message.insert(cipher_message.end(), ciphertext.begin(), ciphertext.end());
    cipher_message.insert(cipher_message.end(), tag.begin(), tag.end());

    return base64_encode(cipher_message);
}

int main() {
    try {
        // IMPORTANT: In a real-world application, the key must be securely
        // managed and not hardcoded or generated on the fly like this.
        // This is for demonstration purposes only.
        std::vector<unsigned char> key = generate_key();

        std::vector<std::string> testCases = {
            "This is a secret message.",
            "1234567890",
            "!@#$%^&*()_+",
            "A very long message to test the encryption with a larger payload to see how it performs and handles bigger data chunks.",
            "" // Empty string
        };

        std::cout << "Using AES Key (Base64): " << base64_encode(key) << std::endl;
        std::cout << "--- Running 5 Test Cases ---" << std::endl;

        for (int i = 0; i < testCases.size(); ++i) {
            const std::string& plaintext = testCases[i];
            std::cout << "\nTest Case " << (i + 1) << std::endl;
            std::cout << "Plaintext: " << plaintext << std::endl;
            std::string encryptedText = encrypt(plaintext, key);
            std::cout << "Encrypted (Base64): " << encryptedText << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}