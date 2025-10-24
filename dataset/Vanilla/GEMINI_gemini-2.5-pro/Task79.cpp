#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Encrypts a message using the Vigenère cipher.
 * 
 * @param message The plaintext message to encrypt.
 * @param key The secret key for encryption.
 * @return The encrypted ciphertext.
 */
std::string encrypt(const std::string& message, const std::string& key) {
    if (key.empty()) {
        return message;
    }

    std::string encryptedMessage = "";
    std::string upperKey;
    for (char c : key) {
        upperKey += toupper(c);
    }

    int keyIndex = 0;
    for (size_t i = 0; i < message.length(); ++i) {
        char messageChar = message[i];
        if (isalpha(messageChar)) {
            char keyChar = upperKey[keyIndex % upperKey.length()];
            int shift = keyChar - 'A';
            char encryptedChar = ((toupper(messageChar) - 'A' + shift) % 26) + 'A';
            encryptedMessage += encryptedChar;
            keyIndex++;
        } else {
            encryptedMessage += messageChar; // Preserve original non-letter characters
        }
    }
    return encryptedMessage;
}

int main() {
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"ATTACK AT DAWN", "LEMON"},
        {"HELLO WORLD", "KEY"},
        {"Cryptography", "SECRET"},
        {"This is a test message 123!", "CIPHER"},
        {"ALLCAPS", "ALLCAPS"}
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        std::string message = test_cases[i].first;
        std::string key = test_cases[i].second;
        std::string encrypted = encrypt(message, key);

        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "  Original:  " << message << std::endl;
        std::cout << "  Key:       " << key << std::endl;
        std::cout << "  Encrypted: " << encrypted << std::endl;
        std::cout << std::endl;
    }

    return 0;
}