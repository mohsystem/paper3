
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <vector>

std::string base64_encode(const unsigned char* buffer, size_t length) {
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

std::string encryptAES(const std::string& plainText, const std::string& key) {
    unsigned char keyBytes[16] = {0};
    memcpy(keyBytes, key.c_str(), std::min(key.length(), (size_t)16));
    
    unsigned char iv[16];
    RAND_bytes(iv, 16);
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, keyBytes, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    std::vector<unsigned char> encrypted(plainText.length() + AES_BLOCK_SIZE);
    int len;
    int ciphertext_len;
    
    if (EVP_EncryptUpdate(ctx, encrypted.data(), &len, 
                          (unsigned char*)plainText.c_str(), plainText.length()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    std::vector<unsigned char> combined(16 + ciphertext_len);
    memcpy(combined.data(), iv, 16);
    memcpy(combined.data() + 16, encrypted.data(), ciphertext_len);
    
    return base64_encode(combined.data(), combined.size());
}

int main() {
    // Test case 1
    std::string encrypted1 = encryptAES("Hello World", "mySecretKey12345");
    std::cout << "Test 1 - Encrypted: " << encrypted1 << std::endl;
    
    // Test case 2
    std::string encrypted2 = encryptAES("Sensitive Data", "password1234567");
    std::cout << "Test 2 - Encrypted: " << encrypted2 << std::endl;
    
    // Test case 3
    std::string encrypted3 = encryptAES("AES Encryption", "key123456789012");
    std::cout << "Test 3 - Encrypted: " << encrypted3 << std::endl;
    
    // Test case 4
    std::string encrypted4 = encryptAES("Test Message", "secretKey123456");
    std::cout << "Test 4 - Encrypted: " << encrypted4 << std::endl;
    
    // Test case 5
    std::string encrypted5 = encryptAES("Cipher Block", "encryptionKey16");
    std::cout << "Test 5 - Encrypted: " << encrypted5 << std::endl;
    
    return 0;
}
