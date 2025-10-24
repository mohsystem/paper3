
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class Task51 {
private:
    static const int AES_KEY_SIZE = 32;
    static const int GCM_IV_SIZE = 12;
    static const int GCM_TAG_SIZE = 16;

    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (length--) {
            char_array_3[i++] = *(buffer++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];

            while(i++ < 3)
                ret += '=';
        }

        return ret;
    }

    static std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<unsigned char> ret;

        while (in_len-- && (encoded_string[in_] != '=') && 
               (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; i < 3; i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i) {
            for (j = 0; j < i; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (j = 0; j < i - 1; j++)
                ret.push_back(char_array_3[j]);
        }

        return ret;
    }

public:
    static std::string generateKey() {
        unsigned char key[AES_KEY_SIZE];
        if (RAND_bytes(key, AES_KEY_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }
        return base64_encode(key, AES_KEY_SIZE);
    }

    static std::string encrypt(const std::string& plaintext, const std::string& base64Key) {
        if (plaintext.empty()) {
            throw std::invalid_argument("Plaintext cannot be empty");
        }
        if (base64Key.empty()) {
            throw std::invalid_argument("Key cannot be empty");
        }

        std::vector<unsigned char> key = base64_decode(base64Key);
        
        unsigned char iv[GCM_IV_SIZE];
        if (RAND_bytes(iv, GCM_IV_SIZE) != 1) {
            throw std::runtime_error("Failed to generate IV");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        std::vector<unsigned char> ciphertext(plaintext.length() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len;
        int ciphertext_len;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                              (unsigned char*)plaintext.c_str(), plaintext.length()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;

        unsigned char tag[GCM_TAG_SIZE];
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to get tag");
        }

        EVP_CIPHER_CTX_free(ctx);

        std::vector<unsigned char> result;
        result.insert(result.end(), iv, iv + GCM_IV_SIZE);
        result.insert(result.end(), tag, tag + GCM_TAG_SIZE);
        result.insert(result.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);

        return base64_encode(result.data(), result.size());
    }

    static std::string decrypt(const std::string& encryptedText, const std::string& base64Key) {
        if (encryptedText.empty()) {
            throw std::invalid_argument("Encrypted text cannot be empty");
        }
        if (base64Key.empty()) {
            throw std::invalid_argument("Key cannot be empty");
        }

        std::vector<unsigned char> key = base64_decode(base64Key);
        std::vector<unsigned char> encrypted = base64_decode(encryptedText);

        if (encrypted.size() < GCM_IV_SIZE + GCM_TAG_SIZE) {
            throw std::runtime_error("Invalid encrypted data");
        }

        unsigned char* iv = encrypted.data();
        unsigned char* tag = encrypted.data() + GCM_IV_SIZE;
        unsigned char* ciphertext = encrypted.data() + GCM_IV_SIZE + GCM_TAG_SIZE;
        int ciphertext_len = encrypted.size() - GCM_IV_SIZE - GCM_TAG_SIZE;

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        std::vector<unsigned char> plaintext(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        int len;
        int plaintext_len;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, ciphertext_len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption failed");
        }
        plaintext_len = len;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set tag");
        }

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption verification failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        return std::string((char*)plaintext.data(), plaintext_len);
    }
};

int main() {
    try {
        std::string key = Task51::generateKey();
        std::cout << "Generated Key: " << key << std::endl << std::endl;

        // Test Case 1
        std::string plaintext1 = "Hello, World!";
        std::string encrypted1 = Task51::encrypt(plaintext1, key);
        std::string decrypted1 = Task51::decrypt(encrypted1, key);
        std::cout << "Test Case 1:" << std::endl;
        std::cout << "Original: " << plaintext1 << std::endl;
        std::cout << "Encrypted: " << encrypted1 << std::endl;
        std::cout << "Decrypted: " << decrypted1 << std::endl;
        std::cout << "Match: " << (plaintext1 == decrypted1 ? "true" : "false") << std::endl << std::endl;

        // Test Case 2
        std::string plaintext2 = "Secure Encryption 2024!";
        std::string encrypted2 = Task51::encrypt(plaintext2, key);
        std::string decrypted2 = Task51::decrypt(encrypted2, key);
        std::cout << "Test Case 2:" << std::endl;
        std::cout << "Original: " << plaintext2 << std::endl;
        std::cout << "Encrypted: " << encrypted2 << std::endl;
        std::cout << "Decrypted: " << decrypted2 << std::endl;
        std::cout << "Match: " << (plaintext2 == decrypted2 ? "true" : "false") << std::endl << std::endl;

        // Test Case 3
        std::string plaintext3 = "Special characters: @#$%^&*()";
        std::string encrypted3 = Task51::encrypt(plaintext3, key);
        std::string decrypted3 = Task51::decrypt(encrypted3, key);
        std::cout << "Test Case 3:" << std::endl;
        std::cout << "Original: " << plaintext3 << std::endl;
        std::cout << "Encrypted: " << encrypted3 << std::endl;
        std::cout << "Decrypted: " << decrypted3 << std::endl;
        std::cout << "Match: " << (plaintext3 == decrypted3 ? "true" : "false") << std::endl << std::endl;

        // Test Case 4
        std::string plaintext4 = "1234567890";
        std::string encrypted4 = Task51::encrypt(plaintext4, key);
        std::string decrypted4 = Task51::decrypt(encrypted4, key);
        std::cout << "Test Case 4:" << std::endl;
        std::cout << "Original: " << plaintext4 << std::endl;
        std::cout << "Encrypted: " << encrypted4 << std::endl;
        std::cout << "Decrypted: " << decrypted4 << std::endl;
        std::cout << "Match: " << (plaintext4 == decrypted4 ? "true" : "false") << std::endl << std::endl;

        // Test Case 5
        std::string plaintext5 = "This is a longer text to test encryption with multiple words and sentences.";
        std::string encrypted5 = Task51::encrypt(plaintext5, key);
        std::string decrypted5 = Task51::decrypt(encrypted5, key);
        std::cout << "Test Case 5:" << std::endl;
        std::cout << "Original: " << plaintext5 << std::endl;
        std::cout << "Encrypted: " << encrypted5 << std::endl;
        std::cout << "Decrypted: " << decrypted5 << std::endl;
        std::cout << "Match: " << (plaintext5 == decrypted5 ? "true" : "false") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
