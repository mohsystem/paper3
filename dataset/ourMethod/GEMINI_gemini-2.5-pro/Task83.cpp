#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Constants based on security rules
constexpr int SALT_LENGTH_BYTES = 16;
constexpr int IV_LENGTH_BYTES = 12;
constexpr int TAG_LENGTH_BYTES = 16; // 128 bits
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr int AES_KEY_BITS = 256;
constexpr int AES_KEY_BYTES = AES_KEY_BITS / 8;

// RAII wrappers for OpenSSL resources
struct EVP_CIPHER_CTX_deleter {
    void operator()(EVP_CIPHER_CTX* ptr) const {
        if (ptr) {
            EVP_CIPHER_CTX_free(ptr);
        }
    }
};
using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_deleter>;

// Helper to throw a standardized exception on OpenSSL errors
void handle_openssl_error() {
    throw std::runtime_error("OpenSSL error.");
}

// Helper to Base64 encode data for printing
std::string to_base64(const std::vector<unsigned char>& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) handle_openssl_error();
    BIO* b64 = BIO_new(BIO_f_base64());
    if (!b64) { BIO_free(bio); handle_openssl_error(); }
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    return result;
}

/**
 * Encrypts plaintext using AES-256-GCM, deriving the key from a password.
 * The security rules mandate using AES-GCM over the requested AES-CBC.
 */
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext, const std::string& password) {
    // 1. Generate a random salt
    std::vector<unsigned char> salt(SALT_LENGTH_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) handle_openssl_error();

    // 2. Derive the key from the password and salt
    std::vector<unsigned char> key(AES_KEY_BYTES);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITERATIONS, EVP_sha256(), key.size(), key.data()) != 1) handle_openssl_error();

    // 3. Generate a random IV
    std::vector<unsigned char> iv(IV_LENGTH_BYTES);
    if (RAND_bytes(iv.data(), iv.size()) != 1) handle_openssl_error();

    // 4. Encrypt the plaintext
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) handle_openssl_error();
    
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) handle_openssl_error();
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr) != 1) handle_openssl_error();
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1) handle_openssl_error();

    std::vector<unsigned char> ciphertext(plaintext.size());
    int len = 0;
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) handle_openssl_error();
    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) handle_openssl_error();
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    std::vector<unsigned char> tag(TAG_LENGTH_BYTES);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data()) != 1) handle_openssl_error();

    // 5. Concatenate salt, IV, ciphertext, and tag
    std::vector<unsigned char> encrypted_data;
    encrypted_data.reserve(salt.size() + iv.size() + ciphertext.size() + tag.size());
    encrypted_data.insert(encrypted_data.end(), salt.begin(), salt.end());
    encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
    encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());
    encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());
    
    OPENSSL_cleanse(key.data(), key.size());

    return encrypted_data;
}

int main() {
    // NOTE: Hardcoding secrets is insecure and violates security rules.
    // This is for demonstration purposes only. In a real application,
    // use a secure mechanism like environment variables or a secrets vault.
    std::string password = "very-strong-password-123";
    std::vector<std::string> test_keys_to_encrypt = {
        "0123456789abcdef0123456789abcdef",
        "short key",
        "a-longer-key-with-some-symbols!@#$",
        "", // Empty string
        "another key for testing purposes 12345"
    };

    std::cout << "Running 5 test cases for AES-GCM encryption..." << std::endl;

    for (size_t i = 0; i < test_keys_to_encrypt.size(); ++i) {
        try {
            std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
            const std::string& original_key = test_keys_to_encrypt[i];
            std::vector<unsigned char> plaintext(original_key.begin(), original_key.end());
            std::cout << "Original data: " << original_key << std::endl;

            std::vector<unsigned char> encrypted_data = encrypt(plaintext, password);
            std::cout << "Encrypted (Base64): " << to_base64(encrypted_data) << std::endl;
            std::cout << "Encrypted data length: " << encrypted_data.size() << " bytes" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Test Case " << (i + 1) << " failed: " << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
        }
    }

    return 0;
}