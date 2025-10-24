#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>

constexpr int SALT_LENGTH_BYTES = 16;
constexpr int IV_LENGTH_BYTES = 12;
constexpr int TAG_LENGTH_BYTES = 16;
constexpr int KEY_LENGTH_BYTES = 32; // 256 bits
constexpr int PBKDF2_ITERATIONS = 600000;

// Helper to convert byte vector to hex string for printing
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<unsigned char> encrypt(const std::string& plaintext, const std::string& passphrase) {
    if (plaintext.empty() || passphrase.empty()) {
        throw std::invalid_argument("Plaintext and passphrase must not be empty.");
    }

    std::vector<unsigned char> salt(SALT_LENGTH_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate salt.");
    }

    std::vector<unsigned char> iv(IV_LENGTH_BYTES);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate IV.");
    }

    std::vector<unsigned char> key(KEY_LENGTH_BYTES);
    if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), passphrase.length(),
                          salt.data(), salt.size(), PBKDF2_ITERATIONS,
                          EVP_sha256(), key.size(), key.data()) != 1) {
        throw std::runtime_error("Failed to derive key.");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context.");
    }

    std::vector<unsigned char> ciphertext(plaintext.length());
    std::vector<unsigned char> tag(TAG_LENGTH_BYTES);
    int len = 0;
    int ciphertext_len = 0;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption.");
    }
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set IV length.");
    }
    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set key and IV.");
    }
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                               reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt plaintext.");
    }
    ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption.");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get GCM tag.");
    }

    EVP_CIPHER_CTX_free(ctx);

    std::vector<unsigned char> result;
    result.reserve(salt.size() + iv.size() + ciphertext.size() + tag.size());
    result.insert(result.end(), salt.begin(), salt.end());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    result.insert(result.end(), tag.begin(), tag.end());
    
    return result;
}

std::optional<std::string> decrypt(const std::vector<unsigned char>& ciphertext_blob, const std::string& passphrase) {
    if (passphrase.empty() || ciphertext_blob.size() < SALT_LENGTH_BYTES + IV_LENGTH_BYTES + TAG_LENGTH_BYTES) {
        return std::nullopt;
    }

    const unsigned char* p = ciphertext_blob.data();
    std::vector<unsigned char> salt(p, p + SALT_LENGTH_BYTES);
    p += SALT_LENGTH_BYTES;
    std::vector<unsigned char> iv(p, p + IV_LENGTH_BYTES);
    p += IV_LENGTH_BYTES;
    std::vector<unsigned char> ciphertext(p, p + ciphertext_blob.size() - SALT_LENGTH_BYTES - IV_LENGTH_BYTES - TAG_LENGTH_BYTES);
    p += ciphertext.size();
    std::vector<unsigned char> tag(p, p + TAG_LENGTH_BYTES);

    std::vector<unsigned char> key(KEY_LENGTH_BYTES);
    if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), passphrase.length(),
                          salt.data(), salt.size(), PBKDF2_ITERATIONS,
                          EVP_sha256(), key.size(), key.data()) != 1) {
        return std::nullopt; // Key derivation failed
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return std::nullopt;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    
    int len;
    std::vector<unsigned char> plaintext(ciphertext.size());
    int plaintext_len = 0;

    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    plaintext_len = len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    
    // Final check happens here
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);

    EVP_CIPHER_CTX_free(ctx);
    
    if (ret > 0) {
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext.size());
    } else {
        // Authentication failed
        return std::nullopt;
    }
}

void run_test_case(int i, const std::string& original_data, const std::string& passphrase) {
    std::cout << "--- Test Case " << i << " ---" << std::endl;
    std::cout << "Original: " << original_data << std::endl;
    std::cout << "Passphrase: " << passphrase << std::endl;

    try {
        std::vector<unsigned char> encrypted_data = encrypt(original_data, passphrase);
        std::cout << "Encrypted (Hex): " << to_hex(encrypted_data) << std::endl;

        std::optional<std::string> decrypted_data = decrypt(encrypted_data, passphrase);
        if (decrypted_data) {
            std::cout << "Decrypted (Success): " << *decrypted_data << std::endl;
            if (*decrypted_data == original_data) {
                std::cout << "SUCCESS: Decrypted data matches original." << std::endl;
            } else {
                std::cout << "FAILURE: Decrypted data does NOT match original." << std::endl;
            }
        } else {
            std::cout << "FAILURE: Decryption failed with correct passphrase." << std::endl;
        }

        std::optional<std::string> failed_decryption = decrypt(encrypted_data, "wrong-password");
        if (!failed_decryption) {
            std::cout << "SUCCESS: Decryption failed as expected with wrong passphrase." << std::endl;
        } else {
            std::cout << "FAILURE: Decryption succeeded with wrong passphrase." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::string test_passphrases[] = {
        "correct-horse-battery-staple", "Tr0ub4dor&3", "!@#$%^&*()_+",
        "a long passphrase with spaces and punctuation", "short"
    };
    std::string test_data[] = {
        "Credit Card: 1234-5678-9012-3456", "SSN: 000-00-0000",
        "{\"user\":\"admin\",\"password\":\"password123\"}", "This is a secret message.",
        "Another piece of sensitive info."
    };

    for (int i = 0; i < 5; ++i) {
        run_test_case(i + 1, test_data[i], test_passphrases[i]);
    }

    return 0;
}