
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class EncryptionResult {
private:
    std::vector<unsigned char> encryptedData;
    std::vector<unsigned char> iv;

public:
    EncryptionResult(const std::vector<unsigned char>& data, const std::vector<unsigned char>& ivData)
        : encryptedData(data), iv(ivData) {}

    const std::vector<unsigned char>& getEncryptedData() const { return encryptedData; }
    const std::vector<unsigned char>& getIv() const { return iv; }

    std::string toBase64() const {
        return base64Encode(encryptedData);
    }

    std::string getIvBase64() const {
        return base64Encode(iv);
    }

private:
    static std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char base64Chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64Chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        
        if (valb > -6) {
            result.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        
        while (result.size() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
};

class Task83 {
public:
    static EncryptionResult encryptData(const std::vector<unsigned char>& plaintext, 
                                       const std::vector<unsigned char>& keyBytes) {
        if (plaintext.empty()) {
            throw std::invalid_argument("Plaintext cannot be empty");
        }
        
        if (keyBytes.size() != 16 && keyBytes.size() != 24 && keyBytes.size() != 32) {
            throw std::invalid_argument("Key must be 16, 24, or 32 bytes");
        }

        // Generate random IV
        std::vector<unsigned char> iv(16);
        if (RAND_bytes(iv.data(), 16) != 1) {
            throw std::runtime_error("Failed to generate random IV");
        }

        // Create and initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        const EVP_CIPHER* cipher;
        if (keyBytes.size() == 16) {
            cipher = EVP_aes_128_cbc();
        } else if (keyBytes.size() == 24) {
            cipher = EVP_aes_192_cbc();
        } else {
            cipher = EVP_aes_256_cbc();
        }

        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, keyBytes.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        // Allocate buffer for encrypted data
        std::vector<unsigned char> encryptedData(plaintext.size() + EVP_CIPHER_block_size(cipher));
        int len;
        int ciphertext_len;

        // Encrypt plaintext
        if (EVP_EncryptUpdate(ctx, encryptedData.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        ciphertext_len = len;

        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;

        encryptedData.resize(ciphertext_len);
        EVP_CIPHER_CTX_free(ctx);

        return EncryptionResult(encryptedData, iv);
    }

    static std::vector<unsigned char> generateSecureKey(int keySize) {
        if (keySize != 128 && keySize != 192 && keySize != 256) {
            throw std::invalid_argument("Key size must be 128, 192, or 256 bits");
        }

        int keyBytes = keySize / 8;
        std::vector<unsigned char> key(keyBytes);
        
        if (RAND_bytes(key.data(), keyBytes) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }

        return key;
    }
};

int main() {
    try {
        std::cout << "AES CBC Encryption Test Cases\\n\\n";

        // Test Case 1: Standard encryption with 256-bit key
        std::cout << "Test Case 1: 256-bit key encryption\\n";
        auto key1 = Task83::generateSecureKey(256);
        std::string plaintext1 = "Hello, Secure World!";
        std::vector<unsigned char> data1(plaintext1.begin(), plaintext1.end());
        auto result1 = Task83::encryptData(data1, key1);
        std::cout << "Plaintext: " << plaintext1 << "\\n";
        std::cout << "Encrypted (Base64): " << result1.toBase64() << "\\n";
        std::cout << "IV (Base64): " << result1.getIvBase64() << "\\n\\n";

        // Test Case 2: Encryption with 128-bit key
        std::cout << "Test Case 2: 128-bit key encryption\\n";
        auto key2 = Task83::generateSecureKey(128);
        std::string plaintext2 = "Sensitive Data 123";
        std::vector<unsigned char> data2(plaintext2.begin(), plaintext2.end());
        auto result2 = Task83::encryptData(data2, key2);
        std::cout << "Plaintext: " << plaintext2 << "\\n";
        std::cout << "Encrypted (Base64): " << result2.toBase64() << "\\n";
        std::cout << "IV (Base64): " << result2.getIvBase64() << "\\n\\n";

        // Test Case 3: Large data encryption
        std::cout << "Test Case 3: Large data encryption\\n";
        auto key3 = Task83::generateSecureKey(256);
        std::string plaintext3 = "This is a much longer message that contains multiple sentences. "
                                "It demonstrates the encryption of larger data blocks using AES CBC mode.";
        std::vector<unsigned char> data3(plaintext3.begin(), plaintext3.end());
        auto result3 = Task83::encryptData(data3, key3);
        std::cout << "Plaintext length: " << plaintext3.length() << " bytes\\n";
        std::cout << "Encrypted (Base64): " << result3.toBase64() << "\\n";
        std::cout << "IV (Base64): " << result3.getIvBase64() << "\\n\\n";

        // Test Case 4: Special characters encryption
        std::cout << "Test Case 4: Special characters encryption\\n";
        auto key4 = Task83::generateSecureKey(192);
        std::string plaintext4 = "Special chars: @#$%^&*()_+-={}[]|:;<>?,./~`";
        std::vector<unsigned char> data4(plaintext4.begin(), plaintext4.end());
        auto result4 = Task83::encryptData(data4, key4);
        std::cout << "Plaintext: " << plaintext4 << "\\n";
        std::cout << "Encrypted (Base64): " << result4.toBase64() << "\\n";
        std::cout << "IV (Base64): " << result4.getIvBase64() << "\\n\\n";

        // Test Case 5: Verify different IVs produce different ciphertexts
        std::cout << "Test Case 5: Same plaintext, different IVs\\n";
        auto key5 = Task83::generateSecureKey(256);
        std::string plaintext5 = "Same message";
        std::vector<unsigned char> data5(plaintext5.begin(), plaintext5.end());
        auto result5a = Task83::encryptData(data5, key5);
        auto result5b = Task83::encryptData(data5, key5);
        std::cout << "Plaintext: " << plaintext5 << "\\n";
        std::cout << "Encrypted 1 (Base64): " << result5a.toBase64() << "\\n";
        std::cout << "Encrypted 2 (Base64): " << result5b.toBase64() << "\\n";
        std::cout << "Are ciphertexts different? " 
                  << (result5a.getEncryptedData() != result5b.getEncryptedData() ? "true" : "false") << "\\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
