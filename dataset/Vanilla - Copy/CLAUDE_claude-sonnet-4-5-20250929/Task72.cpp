
#include <iostream>
#include <vector>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

const unsigned char STATIC_IV[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const int BLOCK_SIZE = 16;

std::vector<unsigned char> padData(const std::vector<unsigned char>& data, int blockSize) {
    int paddingLength = blockSize - (data.size() % blockSize);
    if (paddingLength == 0) {
        paddingLength = blockSize;
    }
    
    std::vector<unsigned char> paddedData = data;
    for (int i = 0; i < paddingLength; i++) {
        paddedData.push_back((unsigned char)paddingLength);
    }
    
    return paddedData;
}

std::vector<unsigned char> encryptData(const std::vector<unsigned char>& data, const unsigned char* key) {
    std::vector<unsigned char> paddedData = padData(data, BLOCK_SIZE);
    std::vector<unsigned char> encryptedData(paddedData.size());
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    int len;
    EVP_EncryptUpdate(ctx, encryptedData.data(), &len, paddedData.data(), paddedData.size());
    
    int finalLen;
    EVP_EncryptFinal_ex(ctx, encryptedData.data() + len, &finalLen);
    
    EVP_CIPHER_CTX_free(ctx);
    
    return encryptedData;
}

std::string toBase64(const std::vector<unsigned char>& data) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string result;
    int i = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    
    for (size_t j = 0; j < data.size(); j++) {
        array3[i++] = data[j];
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                result += base64_chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 3; j++)
            array3[j] = '\\0';
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            result += base64_chars[array4[j]];
        
        while (i++ < 3)
            result += '=';
    }
    
    return result;
}

int main() {
    unsigned char key[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    
    std::cout << "Test Case 1:" << std::endl;
    std::string str1 = "Hello World!";
    std::vector<unsigned char> data1(str1.begin(), str1.end());
    std::vector<unsigned char> encrypted1 = encryptData(data1, key);
    std::cout << "Encrypted: " << toBase64(encrypted1) << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::string str2 = "Test";
    std::vector<unsigned char> data2(str2.begin(), str2.end());
    std::vector<unsigned char> encrypted2 = encryptData(data2, key);
    std::cout << "Encrypted: " << toBase64(encrypted2) << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::string str3 = "A";
    std::vector<unsigned char> data3(str3.begin(), str3.end());
    std::vector<unsigned char> encrypted3 = encryptData(data3, key);
    std::cout << "Encrypted: " << toBase64(encrypted3) << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::string str4 = "This is a longer message for encryption testing";
    std::vector<unsigned char> data4(str4.begin(), str4.end());
    std::vector<unsigned char> encrypted4 = encryptData(data4, key);
    std::cout << "Encrypted: " << toBase64(encrypted4) << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::string str5 = "1234567890123456";
    std::vector<unsigned char> data5(str5.begin(), str5.end());
    std::vector<unsigned char> encrypted5 = encryptData(data5, key);
    std::cout << "Encrypted: " << toBase64(encrypted5) << std::endl;
    
    return 0;
}
