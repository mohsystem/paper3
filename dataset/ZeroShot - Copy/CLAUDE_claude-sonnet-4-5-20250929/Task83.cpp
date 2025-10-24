
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <vector>

std::string base64_encode(const unsigned char* buffer, size_t length) {
    static const char* base64_chars = 
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

            for(i = 0; (i <4) ; i++)
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

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::string encryptAES(const std::string& key, const std::string& plainText) {
    // Prepare key (ensure it's 16 bytes)
    unsigned char keyBytes[16];
    memset(keyBytes, 0, 16);
    memcpy(keyBytes, key.c_str(), std::min(key.length(), size_t(16)));
    
    // Generate random IV
    unsigned char iv[16];
    RAND_bytes(iv, 16);
    
    // Initialize cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, keyBytes, iv);
    
    // Encrypt
    int len;
    int ciphertext_len;
    unsigned char ciphertext[1024];
    
    EVP_EncryptUpdate(ctx, ciphertext, &len, 
                      (unsigned char*)plainText.c_str(), plainText.length());
    ciphertext_len = len;
    
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Combine IV and ciphertext
    std::vector<unsigned char> combined(16 + ciphertext_len);
    memcpy(&combined[0], iv, 16);
    memcpy(&combined[16], ciphertext, ciphertext_len);
    
    // Return Base64 encoded result
    return base64_encode(&combined[0], combined.size());
}

int main() {
    std::string keys[] = {
        "mySecretKey12345",
        "anotherKey456789",
        "testKey123456789",
        "secureKey9876543",
        "randomKey1122334"
    };
    
    std::string plainTexts[] = {
        "Hello World",
        "This is a test message",
        "AES encryption in CBC mode",
        "Secure data transmission",
        "Testing encryption"
    };
    
    for (int i = 0; i < 5; i++) {
        std::string encrypted = encryptAES(keys[i], plainTexts[i]);
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Key: " << keys[i] << std::endl;
        std::cout << "Plain Text: " << plainTexts[i] << std::endl;
        std::cout << "Encrypted: " << encrypted << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
