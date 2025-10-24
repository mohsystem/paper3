#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>

// Constants
constexpr int AES_KEY_SIZE_BYTES = 32;    // 256-bit
constexpr int GCM_IV_LENGTH_BYTES = 12;
constexpr int GCM_TAG_LENGTH_BYTES = 16;
constexpr int SALT_LENGTH_BYTES = 16;
constexpr int PBKDF2_ITERATIONS = 600000;

// Custom deleter for EVP_CIPHER_CTX
struct EVPCipherCtxDeleter {
    void operator()(EVP_CIPHER_CTX* ctx) const {
        if (ctx) EVP_CIPHER_CTX_free(ctx);
    }
};
using EVPCipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, EVPCipherCtxDeleter>;

std::vector<unsigned char> derive_key(const std::string& passphrase, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> key(AES_KEY_SIZE_BYTES);
    if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), passphrase.length(),
                          salt.data(), salt.size(),
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          key.size(), key.data()) != 1) {
        throw std::runtime_error("Key derivation failed");
    }
    return key;
}

std::vector<unsigned char> encrypt(const std::string& plaintext, const std::string& passphrase) {
    std::vector<unsigned char> salt(SALT_LENGTH_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    auto key = derive_key(passphrase, salt);

    std::vector<unsigned char> iv(GCM_IV_LENGTH_BYTES);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate IV");
    }

    EVPCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) {
        throw std::runtime_error("Failed to initialize encryption");
    }

    std::vector<unsigned char> ciphertext(plaintext.length());
    int len = 0;
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length()) != 1) {
        throw std::runtime_error("Encryption update failed");
    }
    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        throw std::runtime_error("Encryption finalization failed");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    std::vector<unsigned char> tag(GCM_TAG_LENGTH_BYTES);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH_BYTES, tag.data()) != 1) {
        throw std::runtime_error("Failed to get GCM tag");
    }

    OPENSSL_cleanse(key.data(), key.size());

    std::vector<unsigned char> encrypted_data;
    encrypted_data.reserve(salt.size() + iv.size() + ciphertext.size() + tag.size());
    encrypted_data.insert(encrypted_data.end(), salt.begin(), salt.end());
    encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
    encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());
    encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());
    
    return encrypted_data;
}

std::string decrypt(const std::vector<unsigned char>& encrypted_data, const std::string& passphrase) {
    if (encrypted_data.size() < SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES + GCM_TAG_LENGTH_BYTES) {
        return ""; // Indicate failure: data too short
    }

    std::vector<unsigned char> salt(encrypted_data.begin(), encrypted_data.begin() + SALT_LENGTH_BYTES);
    std::vector<unsigned char> iv(encrypted_data.begin() + SALT_LENGTH_BYTES, encrypted_data.begin() + SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES);
    std::vector<unsigned char> ciphertext(encrypted_data.begin() + SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES, encrypted_data.end() - GCM_TAG_LENGTH_BYTES);
    std::vector<unsigned char> tag(encrypted_data.end() - GCM_TAG_LENGTH_BYTES, encrypted_data.end());

    auto key = derive_key(passphrase, salt);

    EVPCipherCtxPtr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) return "";

    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) {
        OPENSSL_cleanse(key.data(), key.size()); return "";
    }

    std::vector<unsigned char> plaintext(ciphertext.size());
    int len = 0;
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        OPENSSL_cleanse(key.data(), key.size()); return "";
    }
    int plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH_BYTES, tag.data()) != 1) {
        OPENSSL_cleanse(key.data(), key.size()); return "";
    }
    
    int ret = EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len);
    OPENSSL_cleanse(key.data(), key.size());

    if (ret > 0) {
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        return std::string(reinterpret_cast<char*>(plaintext.data()), plaintext.size());
    } else {
        return ""; // Decryption failed (e.g., tag mismatch)
    }
}

std::string to_base64(const std::vector<unsigned char>& data); // Forward declaration

void run_test(const std::string& original_plaintext, const std::string& passphrase, int test_num) {
    std::cout << "\n--- Test Case " << test_num << " ---" << std::endl;
    std::cout << "Original:  " << original_plaintext << std::endl;
    
    try {
        std::vector<unsigned char> encrypted_data = encrypt(original_plaintext, passphrase);
        std::cout << "Encrypted (Base64): " << to_base64(encrypted_data) << std::endl;
        std::string decrypted_plaintext = decrypt(encrypted_data, passphrase);
        std::cout << "Decrypted: " << decrypted_plaintext << std::endl;
        std::cout << "Result: " << (original_plaintext == decrypted_plaintext ? "SUCCESS" : "FAILURE") << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "Result: FAILURE" << std::endl;
    }
}

int main() {
    std::cout << "--- Cryptographic Key Management Demo ---" << std::endl;
    std::string passphrase = "a_very-s3cur3-p@ssphr@se!";
    std::vector<std::string> test_cases = {
        "Hello, World!", "This is a secret message.",
        "Another test case with some special characters: !@#$%^&*()", "Short", ""
    };
    for (size_t i = 0; i < test_cases.size(); ++i) {
        run_test(test_cases[i], passphrase, i + 1);
    }
    
    std::cout << "\n--- Failure Test Cases ---" << std::endl;
    std::string original_text = "Test for failure cases.";
    std::vector<unsigned char> encrypted = encrypt(original_text, passphrase);

    std::cout << "\n1. Decrypting with wrong passphrase..." << std::endl;
    std::string decrypted_wrong_pass = decrypt(encrypted, "wrong-passphrase");
    std::cout << "Decrypted: " << decrypted_wrong_pass << std::endl;
    std::cout << "Result: " << (decrypted_wrong_pass.empty() ? "SUCCESS (Decryption failed as expected)" : "FAILURE") << std::endl;
    
    std::cout << "\n2. Decrypting with tampered ciphertext..." << std::endl;
    std::vector<unsigned char> tampered_encrypted = encrypted;
    tampered_encrypted.back() ^= 0x01;
    std::string decrypted_tampered = decrypt(tampered_encrypted, passphrase);
    std::cout << "Decrypted: " << decrypted_tampered << std::endl;
    std::cout << "Result: " << (decrypted_tampered.empty() ? "SUCCESS (Decryption failed as expected)" : "FAILURE") << std::endl;

    return 0;
}

// Simple Base64 encoder for printing test output
std::string to_base64(const std::vector<unsigned char>& data) {
    const std::string b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3], char_array_4[4];
    size_t in_len = data.size();
    const unsigned char* bytes_to_encode = data.data();

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++) ret += b64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for(int j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        for (int j = 0; (j < i + 1); j++) ret += b64_chars[char_array_4[j]];
        while((i++ < 3)) ret += '=';
    }
    return ret;
}