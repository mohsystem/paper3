#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

/**
 * @brief Encrypts a string using a repeating key XOR cipher and returns the hex-encoded result.
 * This is a simple educational cipher and is NOT secure for real-world use.
 * 
 * @param plaintext The string to encrypt.
 * @param key The encryption key.
 * @return The hex-encoded encrypted string, or the original plaintext if the key is empty.
 */
std::string encrypt(const std::string& plaintext, const std::string& key) {
    if (key.empty() || plaintext.empty()) {
        return plaintext;
    }

    std::stringstream hex_ciphertext;
    for (size_t i = 0; i < plaintext.length(); ++i) {
        int encrypted_char = static_cast<unsigned char>(plaintext[i]) ^ static_cast<unsigned char>(key[i % key.length()]);
        hex_ciphertext << std::hex << std::setw(2) << std::setfill('0') << encrypted_char;
    }

    return hex_ciphertext.str();
}

int main() {
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"Hello, World!", "KEY"},
        {"This is a longer test message.", "SECRET"},
        {"Short", "LONGERKEY"},
        {"12345!@#$%", "NUMSYM"},
        {"Test with empty key", ""}
    };

    std::cout << "C++ Encryption Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::string plaintext = testCase.first;
        std::string key = testCase.second;
        std::string encrypted = encrypt(plaintext, key);
        std::cout << "Plaintext : \"" << plaintext << "\", Key: \"" << key << "\"" << std::endl;
        std::cout << "Encrypted : " << encrypted << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}