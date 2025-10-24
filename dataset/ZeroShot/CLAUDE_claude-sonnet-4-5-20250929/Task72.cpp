
#include <iostream>
#include <vector>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

class Task72 {
private:
    static const int BLOCK_SIZE = 16;
    static unsigned char STATIC_IV[16];
    
    static std::vector<unsigned char> padData(const std::vector<unsigned char>& data) {
        int paddingLength = BLOCK_SIZE - (data.size() % BLOCK_SIZE);
        if (paddingLength == 0) {
            paddingLength = BLOCK_SIZE;
        }
        
        std::vector<unsigned char> paddedData = data;
        for (int i = 0; i < paddingLength; i++) {
            paddedData.push_back(static_cast<unsigned char>(paddingLength));
        }
        
        return paddedData;
    }
    
public:
    static std::vector<unsigned char> encryptData(const std::vector<unsigned char>& data, 
                                                   const unsigned char* key) {
        // Pad the input data
        std::vector<unsigned char> paddedData = padData(data);
        
        // Initialize cipher context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        std::vector<unsigned char> encryptedData(paddedData.size() + BLOCK_SIZE);
        
        int len;
        int ciphertext_len;
        
        // Initialize encryption with AES-128-CBC
        EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, STATIC_IV);
        EVP_CIPHER_CTX_set_padding(ctx, 0); // Disable padding as we've already padded\n        \n        // Encrypt the data\n        EVP_EncryptUpdate(ctx, encryptedData.data(), &len, paddedData.data(), paddedData.size());\n        ciphertext_len = len;\n        \n        // Finalize encryption\n        EVP_EncryptFinal_ex(ctx, encryptedData.data() + len, &len);\n        ciphertext_len += len;\n        \n        // Clean up\n        EVP_CIPHER_CTX_free(ctx);\n        \n        encryptedData.resize(ciphertext_len);\n        return encryptedData;\n    }\n    \n    static std::string toBase64(const std::vector<unsigned char>& data) {\n        std::stringstream ss;\n        ss << std::hex << std::setfill('0');\n        for (unsigned char c : data) {\n            ss << std::setw(2) << static_cast<int>(c);\n        }\n        return ss.str();\n    }\n};\n\nunsigned char Task72::STATIC_IV[16] = {0};\n\nint main() {\n    unsigned char key[16] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6'};
    
    // Test case 1
    std::string input1 = "Hello World!";
    std::vector<unsigned char> data1(input1.begin(), input1.end());
    std::vector<unsigned char> encrypted1 = Task72::encryptData(data1, key);
    std::cout << "Test 1 - Input: " << input1 << std::endl;
    std::cout << "Encrypted: " << Task72::toBase64(encrypted1) << std::endl;
    
    // Test case 2
    std::string input2 = "SecureData";
    std::vector<unsigned char> data2(input2.begin(), input2.end());
    std::vector<unsigned char> encrypted2 = Task72::encryptData(data2, key);
    std::cout << "\\nTest 2 - Input: " << input2 << std::endl;
    std::cout << "Encrypted: " << Task72::toBase64(encrypted2) << std::endl;
    
    // Test case 3
    std::string input3 = "A";
    std::vector<unsigned char> data3(input3.begin(), input3.end());
    std::vector<unsigned char> encrypted3 = Task72::encryptData(data3, key);
    std::cout << "\\nTest 3 - Input: " << input3 << std::endl;
    std::cout << "Encrypted: " << Task72::toBase64(encrypted3) << std::endl;
    
    // Test case 4
    std::string input4 = "This is a longer message to test encryption";
    std::vector<unsigned char> data4(input4.begin(), input4.end());
    std::vector<unsigned char> encrypted4 = Task72::encryptData(data4, key);
    std::cout << "\\nTest 4 - Input: " << input4 << std::endl;
    std::cout << "Encrypted: " << Task72::toBase64(encrypted4) << std::endl;
    
    // Test case 5
    std::string input5 = "1234567890123456";
    std::vector<unsigned char> data5(input5.begin(), input5.end());
    std::vector<unsigned char> encrypted5 = Task72::encryptData(data5, key);
    std::cout << "\\nTest 5 - Input: " << input5 << std::endl;
    std::cout << "Encrypted: " << Task72::toBase64(encrypted5) << std::endl;
    
    return 0;
}
