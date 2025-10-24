#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <memory>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Constants
constexpr int SALT_SIZE = 16;
constexpr int IV_SIZE = 12;
constexpr int KEY_SIZE = 32; // 256 bits
constexpr int TAG_SIZE = 16; // 128 bits
constexpr int PBKDF2_ITERATIONS = 210000;

// Helper to print binary data as hex
void print_hex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

// Custom deleter for EVP_CIPHER_CTX
struct EvpCipherCtxDeleter {
    void operator()(EVP_CIPHER_CTX* ptr) const {
        if (ptr) {
            EVP_CIPHER_CTX_free(ptr);
        }
    }
};
using EvpCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, EvpCipherCtxDeleter>;

/**
 * Encrypts a plaintext message using a password.
 *
 * @param plaintext The data to encrypt.
 * @param password  The password to derive the key from.
 * @return A vector containing salt, IV, ciphertext, and tag.
 * @throws std::runtime_error on failure.
 */
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext, const std::string& password) {
    // 1. Generate salt and IV
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        throw std::runtime_error("Failed to generate salt.");
    }

    std::vector<unsigned char> iv(IV_SIZE);
    if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
        throw std::runtime_error("Failed to generate IV.");
    }

    // 2. Derive key
    std::vector<unsigned char> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            PBKDF2_ITERATIONS,
            EVP_sha256(),
            key.size(), key.data()) != 1) {
        throw std::runtime_error("Failed to derive key with PBKDF2.");
    }

    // 3. Encrypt
    EvpCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context.");
    }

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        throw std::runtime_error("Failed to initialize AES-256-GCM encryption.");
    }
    // Set IV length (GCM specific)
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
        throw std::runtime_error("Failed to set IV length.");
    }
    // Set key and IV
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) {
        throw std::runtime_error("Failed to set key and IV.");
    }

    // Provide plaintext
    std::vector<unsigned char> ciphertext(plaintext.size());
    int len = 0;
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        throw std::runtime_error("Encryption update failed.");
    }
    int ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        throw std::runtime_error("Encryption finalization failed.");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // Get the authentication tag
    std::vector<unsigned char> tag(TAG_SIZE);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
        throw std::runtime_error("Failed to get GCM tag.");
    }
    
    // Securely clear key from memory
    OPENSSL_cleanse(key.data(), key.size());

    // 4. Concatenate result: salt || iv || ciphertext || tag
    std::vector<unsigned char> result;
    result.reserve(salt.size() + iv.size() + ciphertext.size() + tag.size());
    result.insert(result.end(), salt.begin(), salt.end());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    result.insert(result.end(), tag.begin(), tag.end());

    return result;
}

int main() {
    std::string password = "aVeryStrongPassword!123";

    std::vector<std::string> test_cases = {
        "Hello, World!",
        "This is a secret message.",
        "", // Empty string
        "Short",
        "A much longer message to test the encryption process with more blocks of data to see how it handles it."
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::string pt_str = test_cases[i];
        std::vector<unsigned char> plaintext(pt_str.begin(), pt_str.end());
        std::cout << "Plaintext: " << pt_str << std::endl;

        try {
            std::vector<unsigned char> encrypted_data = encrypt(plaintext, password);
            print_hex("Encrypted (Hex): ", encrypted_data);
        } catch (const std::runtime_error& e) {
            std::cerr << "Encryption failed: " << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
        }
        std::cout << std::endl;
    }

    return 0;
}