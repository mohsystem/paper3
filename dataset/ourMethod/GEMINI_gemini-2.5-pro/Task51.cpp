#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iomanip>
#include <string_view>

// Constants based on security requirements
const int SALT_SIZE = 16;
const int IV_SIZE = 12;
const int TAG_SIZE = 16;
const int KEY_SIZE = 32; // 256 bits for AES-256
const int PBKDF2_ITERATIONS = 210000;

// Helper to handle OpenSSL errors
void handle_openssl_errors() {
    unsigned long err_code = ERR_get_error();
    if (err_code == 0) {
        throw std::runtime_error("An unspecified OpenSSL error occurred.");
    }
    char err_buf[256];
    ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
    throw std::runtime_error("OpenSSL error: " + std::string(err_buf));
}

// RAII wrapper for EVP_CIPHER_CTX
class EvpCipherCtx {
public:
    EvpCipherCtx() : ctx(EVP_CIPHER_CTX_new()) {
        if (!ctx) {
            handle_openssl_errors();
        }
    }
    ~EvpCipherCtx() {
        if (ctx) {
            EVP_CIPHER_CTX_free(ctx);
        }
    }
    EVP_CIPHER_CTX* get() { return ctx; }

    // Disable copy and assignment
    EvpCipherCtx(const EvpCipherCtx&) = delete;
    EvpCipherCtx& operator=(const EvpCipherCtx&) = delete;
private:
    EVP_CIPHER_CTX* ctx;
};

/**
 * Encrypts plaintext using AES-256-GCM.
 * The output format is [salt(16)][iv(12)][ciphertext][tag(16)].
 *
 * @param plaintext The data to encrypt.
 * @param password The password to derive the key from.
 * @return A vector of bytes containing the encrypted data.
 * @throws std::runtime_error on failure.
 */
std::vector<unsigned char> encrypt(std::string_view plaintext, std::string_view password) {
    // 1. Generate a random salt
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        handle_openssl_errors();
    }

    // 2. Derive encryption key from password and salt using PBKDF2-HMAC-SHA256
    std::vector<unsigned char> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(
            password.data(), password.length(),
            salt.data(), salt.size(),
            PBKDF2_ITERATIONS,
            EVP_sha256(),
            key.size(), key.data()) != 1) {
        handle_openssl_errors();
    }

    // 3. Generate a random IV (nonce)
    std::vector<unsigned char> iv(IV_SIZE);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        handle_openssl_errors();
    }

    EvpCipherCtx ctx;
    
    // 4. Initialize AES-256-GCM encryption
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
        handle_openssl_errors();
    }

    std::vector<unsigned char> ciphertext(plaintext.length());
    int len = 0;

    // 5. Encrypt the plaintext
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.length()) != 1) {
        handle_openssl_errors();
    }
    int ciphertext_len = len;
    
    // 6. Finalize the encryption (GCM does not produce extra output here)
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        handle_openssl_errors();
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // 7. Get the GCM authentication tag
    std::vector<unsigned char> tag(TAG_SIZE);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) {
        handle_openssl_errors();
    }

    // 8. Combine salt, IV, ciphertext, and tag into a single buffer
    std::vector<unsigned char> encrypted_data;
    encrypted_data.reserve(salt.size() + iv.size() + ciphertext.size() + tag.size());
    encrypted_data.insert(encrypted_data.end(), salt.begin(), salt.end());
    encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
    encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());
    encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());
    
    // Securely clear the key from memory
    OPENSSL_cleanse(key.data(), key.size());

    return encrypted_data;
}

void print_hex(std::string_view label, const std::vector<unsigned char>& data) {
    std::cout << label << ": ";
    for (const auto& byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

void run_test_case(int case_num, std::string_view plaintext, std::string_view password) {
    std::cout << "--- Test Case " << case_num << " ---" << std::endl;
    if (plaintext.empty()) {
        std::cout << "Plaintext: (empty)" << std::endl;
    } else if (plaintext.length() > 100) {
        std::cout << "Plaintext: (long string, length " << plaintext.length() << ")" << std::endl;
    } else {
        std::cout << "Plaintext: " << plaintext << std::endl;
    }

    try {
        std::vector<unsigned char> encrypted_data = encrypt(plaintext, password);
        print_hex("Encrypted", encrypted_data);
    } catch (const std::exception& e) {
        std::cerr << "Error in Test Case " << case_num << ": " << e.what() << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    run_test_case(1, "This is a secret message.", "strong_password123");
    run_test_case(2, "Another secret.", "another_Pa$$w0rd!");
    run_test_case(3, "", "passwordForEmpty");
    run_test_case(4, std::string(1000, 'A'), "long_and_secure_password_for_a_long_message");
    run_test_case(5, "UTF-8 test: \U0001F600 $¢€", "special-chars-@#$%-password");
    
    return 0;
}