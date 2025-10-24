
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

class Task52 {
private:
    static const int KEY_SIZE = 32;
    static const int IV_SIZE = 16;
    static const int BLOCK_SIZE = 16;

public:
    static std::vector<unsigned char> generateKey(const std::string& password) {
        std::vector<unsigned char> key(KEY_SIZE);
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        std::memcpy(key.data(), hash, KEY_SIZE);
        return key;
    }

    static std::vector<unsigned char> encryptData(const std::vector<unsigned char>& data, 
                                                   const std::string& password) {
        std::vector<unsigned char> key = generateKey(password);
        std::vector<unsigned char> iv(IV_SIZE);
        
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
            throw std::runtime_error("Failed to generate IV");
        }

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                               key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        std::vector<unsigned char> encrypted(data.size() + BLOCK_SIZE);
        int len = 0;
        int ciphertext_len = 0;

        if (EVP_EncryptUpdate(ctx, encrypted.data(), &len, 
                              data.data(), data.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        encrypted.resize(ciphertext_len);
        
        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), encrypted.begin(), encrypted.end());
        
        return result;
    }

    static std::vector<unsigned char> decryptData(const std::vector<unsigned char>& encryptedData, 
                                                   const std::string& password) {
        std::vector<unsigned char> key = generateKey(password);
        std::vector<unsigned char> iv(encryptedData.begin(), 
                                      encryptedData.begin() + IV_SIZE);
        std::vector<unsigned char> ciphertext(encryptedData.begin() + IV_SIZE, 
                                              encryptedData.end());

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                               key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        std::vector<unsigned char> decrypted(ciphertext.size());
        int len = 0;
        int plaintext_len = 0;

        if (EVP_DecryptUpdate(ctx, decrypted.data(), &len, 
                              ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption failed");
        }
        plaintext_len = len;

        if (EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption finalization failed");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        decrypted.resize(plaintext_len);
        return decrypted;
    }

    static void encryptFile(const std::string& inputFile, 
                           const std::string& outputFile, 
                           const std::string& password) {
        std::ifstream in(inputFile, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open input file");
        }

        std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)),
                                        std::istreambuf_iterator<char>());
        in.close();

        std::vector<unsigned char> encrypted = encryptData(data, password);

        std::ofstream out(outputFile, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Cannot open output file");
        }
        out.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
        out.close();
    }

    static void decryptFile(const std::string& inputFile, 
                           const std::string& outputFile, 
                           const std::string& password) {
        std::ifstream in(inputFile, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open input file");
        }

        std::vector<unsigned char> encryptedData((std::istreambuf_iterator<char>(in)),
                                                 std::istreambuf_iterator<char>());
        in.close();

        std::vector<unsigned char> decrypted = decryptData(encryptedData, password);

        std::ofstream out(outputFile, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Cannot open output file");
        }
        out.write(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
        out.close();
    }
};

int main() {
    try {
        // Test Case 1: Simple text encryption/decryption
        std::cout << "Test Case 1: Simple text encryption/decryption\\n";
        std::string text1 = "Hello, World!";
        std::string password1 = "SecurePassword123";
        std::vector<unsigned char> data1(text1.begin(), text1.end());
        auto encrypted1 = Task52::encryptData(data1, password1);
        auto decrypted1 = Task52::decryptData(encrypted1, password1);
        std::string result1(decrypted1.begin(), decrypted1.end());
        std::cout << "Original: " << text1 << "\\n";
        std::cout << "Decrypted: " << result1 << "\\n";
        std::cout << "Match: " << (text1 == result1 ? "true" : "false") << "\\n\\n";

        // Test Case 2: Longer text with special characters
        std::cout << "Test Case 2: Longer text with special characters\\n";
        std::string text2 = "This is a secure message with special chars: @#$%^&*()";
        std::string password2 = "AnotherSecurePass456";
        std::vector<unsigned char> data2(text2.begin(), text2.end());
        auto encrypted2 = Task52::encryptData(data2, password2);
        auto decrypted2 = Task52::decryptData(encrypted2, password2);
        std::string result2(decrypted2.begin(), decrypted2.end());
        std::cout << "Original: " << text2 << "\\n";
        std::cout << "Decrypted: " << result2 << "\\n";
        std::cout << "Match: " << (text2 == result2 ? "true" : "false") << "\\n\\n";

        // Test Case 3: Empty string
        std::cout << "Test Case 3: Empty string\\n";
        std::string text3 = "";
        std::string password3 = "password";
        std::vector<unsigned char> data3(text3.begin(), text3.end());
        auto encrypted3 = Task52::encryptData(data3, password3);
        auto decrypted3 = Task52::decryptData(encrypted3, password3);
        std::string result3(decrypted3.begin(), decrypted3.end());
        std::cout << "Original length: " << text3.length() << "\\n";
        std::cout << "Decrypted length: " << result3.length() << "\\n";
        std::cout << "Match: " << (text3 == result3 ? "true" : "false") << "\\n\\n";

        // Test Case 4: Numeric data
        std::cout << "Test Case 4: Numeric data\\n";
        std::string text4 = "1234567890";
        std::string password4 = "NumericPass999";
        std::vector<unsigned char> data4(text4.begin(), text4.end());
        auto encrypted4 = Task52::encryptData(data4, password4);
        auto decrypted4 = Task52::decryptData(encrypted4, password4);
        std::string result4(decrypted4.begin(), decrypted4.end());
        std::cout << "Original: " << text4 << "\\n";
        std::cout << "Decrypted: " << result4 << "\\n";
        std::cout << "Match: " << (text4 == result4 ? "true" : "false") << "\\n\\n";

        // Test Case 5: File encryption/decryption
        std::cout << "Test Case 5: File encryption/decryption\\n";
        std::string testContent = "This is a test file content for encryption.";
        
        std::ofstream outFile("test_input.txt");
        outFile << testContent;
        outFile.close();

        Task52::encryptFile("test_input.txt", "test_encrypted.bin", "FilePassword123");
        Task52::decryptFile("test_encrypted.bin", "test_decrypted.txt", "FilePassword123");

        std::ifstream inFile("test_decrypted.txt");
        std::string decryptedContent((std::istreambuf_iterator<char>(inFile)),
                                     std::istreambuf_iterator<char>());
        inFile.close();

        std::cout << "Original file content: " << testContent << "\\n";
        std::cout << "Decrypted file content: " << decryptedContent << "\\n";
        std::cout << "Match: " << (testContent == decryptedContent ? "true" : "false") << "\\n";

        // Cleanup
        std::remove("test_input.txt");
        std::remove("test_encrypted.bin");
        std::remove("test_decrypted.txt");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
