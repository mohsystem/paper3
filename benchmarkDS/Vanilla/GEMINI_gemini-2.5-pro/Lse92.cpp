#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

// Note: This code requires the OpenSSL library.
// Compile with: g++ your_file_name.cpp -o your_executable_name -lssl -lcrypto

/**
 * Hashes a password using the SHA-256 algorithm.
 * @param username The username (not used in hashing).
 * @param password The password to hash.
 * @return The SHA-256 hashed password as a hexadecimal string.
 */
std::string registerUser(const std::string& username, const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

int main() {
    // 5 Test Cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"admin", "password123"},
        {"user", "P@ssw0rd!"},
        {"guest", ""},
        {"test_user_long_name", "a_very_long_and_secure_password_with_numbers_12345_and_symbols_!@#$"},
        {"another_user", "password123"}
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        const std::string& username = testCase.first;
        const std::string& password = testCase.second;
        std::string hashedPassword = registerUser(username, password);
        std::cout << "Username: " << username << std::endl;
        std::cout << "Password: " << password << std::endl;
        std::cout << "Hashed Password (SHA-256): " << hashedPassword << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }
    
    return 0;
}