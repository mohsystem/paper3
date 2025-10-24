/*
 * To compile and run this code, you need to have OpenSSL installed.
 *
 * For Debian/Ubuntu: sudo apt-get install libssl-dev
 * For RHEL/CentOS: sudo yum install openssl-devel
 * For macOS (with Homebrew): brew install openssl
 *
 * Compile command:
 * g++ -std=c++17 Task79.cpp -o Task79 -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants
const int AES_KEY_SIZE = 256;
const int AES_KEY_BYTES = AES_KEY_SIZE / 8;
const int GCM_IV_LENGTH = 12; // 96 bits is recommended for GCM
const int GCM_TAG_LENGTH = 16; // 128 bits

// A simple Base64 encode function
std::string base64_encode(const std::vector<unsigned char>& in) {
    const std::string b64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}


/**
 * Encrypts a message using AES-256-GCM.
 * The IV and authentication tag are concatenated with the ciphertext.
 *
 * @param message The plaintext message to encrypt.
 * @param key The 32-byte secret key.
 * @return A Base64 encoded string containing [IV][Ciphertext][Tag].
 */
std::string encrypt(const std::string& message, const unsigned char* key) {
    // 1. Create a Cipher for encryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    // 2. Generate a secure random IV
    std::vector<unsigned char> iv(GCM_IV_LENGTH);
    if (RAND_bytes(iv.data(), GCM_IV_LENGTH) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to generate random IV");
    }

    // 3. Initialize the encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption (1)");
    }
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_LENGTH, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set IV length");
    }
    // Provide key and IV
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption (2)");
    }

    // 4. Encrypt the message
    std::vector<unsigned char> ciphertext(message.length());
    int len = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(message.c_str()), message.length()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    int ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // Get the authentication tag
    std::vector<unsigned char> tag(GCM_TAG_LENGTH);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get GCM tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    // 5. Concatenate IV, ciphertext, and tag
    std::vector<unsigned char> encrypted_data;
    encrypted_data.reserve(GCM_IV_LENGTH + ciphertext.size() + GCM_TAG_LENGTH);
    encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
    encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());
    encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());

    // 6. Encode to Base64
    return base64_encode(encrypted_data);
}

int main() {
    try {
        // Generate a new AES secret key
        unsigned char secret_key[AES_KEY_BYTES];
        if (!RAND_bytes(secret_key, sizeof(secret_key))) {
             throw std::runtime_error("Failed to generate secret key");
        }
        
        std::vector<unsigned char> key_vec(secret_key, secret_key + AES_KEY_BYTES);
        std::cout << "Generated AES-256 Key (Base64): " << base64_encode(key_vec) << std::endl;
        std::cout << "--- Running 5 Test Cases ---" << std::endl;

        std::vector<std::string> test_messages = {
            "This is the first secret message.",
            "Security is important for all applications.",
            "AES-GCM provides authenticated encryption.",
            "1234567890!@#$%^&*()_+",
            "A much longer message to test the encryption process to ensure it works correctly with varying lengths of input data. The quick brown fox jumps over the lazy dog."
        };

        for (int i = 0; i < test_messages.size(); ++i) {
            const std::string& message = test_messages[i];
            std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
            std::cout << "Original Message: " << message << std::endl;
            std::string encrypted_message = encrypt(message, secret_key);
            std::cout << "Encrypted (Base64): " << encrypted_message << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}