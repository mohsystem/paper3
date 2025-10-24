#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <cstdio> // For remove()

// Compile with: g++ -std=c++11 your_file.cpp -o your_app -lssl -lcrypto

const int ITERATIONS = 210000;
const int KEY_LENGTH = 32; // bytes
const int SALT_LENGTH = 16; // bytes

// Helper to convert a vector of bytes to a hex string
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Helper to convert a hex string to a vector of bytes
std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    if (hex.length() % 2 != 0) {
        return bytes;
    }
    for (size_t i = 0; i < hex.length(); i += 2) {
        try {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        } catch (const std::invalid_argument& e) {
            bytes.clear();
            return bytes;
        } catch (const std::out_of_range& e) {
            bytes.clear();
            return bytes;
        }
    }
    return bytes;
}

bool store_password(const std::string& file_path, const std::string& password) {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        return false;
    }

    std::vector<unsigned char> hash(KEY_LENGTH);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         salt.data(), salt.size(), ITERATIONS,
                         EVP_sha256(), hash.size(), hash.data()) != 1) {
        return false;
    }

    std::ofstream outfile(file_path);
    if (!outfile.is_open()) {
        return false;
    }
    outfile << bytes_to_hex(salt) << ":" << bytes_to_hex(hash);
    return true;
}

bool verify_password(const std::string& file_path, const std::string& password) {
    std::ifstream infile(file_path);
    if (!infile.is_open()) {
        return false;
    }

    std::string line;
    if (!std::getline(infile, line)) {
        return false;
    }

    size_t delimiter_pos = line.find(':');
    if (delimiter_pos == std::string::npos) {
        return false;
    }

    std::string salt_hex = line.substr(0, delimiter_pos);
    std::string hash_hex = line.substr(delimiter_pos + 1);

    std::vector<unsigned char> salt = hex_to_bytes(salt_hex);
    std::vector<unsigned char> stored_hash = hex_to_bytes(hash_hex);

    if (salt.size() != SALT_LENGTH || stored_hash.size() != KEY_LENGTH) {
        return false;
    }

    std::vector<unsigned char> test_hash(KEY_LENGTH);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         salt.data(), salt.size(), ITERATIONS,
                         EVP_sha256(), test_hash.size(), test_hash.data()) != 1) {
        return false;
    }
    
    // Constant-time comparison
    return CRYPTO_memcmp(stored_hash.data(), test_hash.data(), KEY_LENGTH) == 0;
}

int main() {
    const std::string test_file_path = "password_storage_cpp.txt";
    const std::string correct_password = "PasswordForC++App1!";

    std::cout << "--- C++ Test Cases for Password Verification ---" << std::endl;

    if (!store_password(test_file_path, correct_password)) {
        std::cerr << "Failed to set up test file. Aborting tests." << std::endl;
        return 1;
    }
    std::cout << "Test file setup complete." << std::endl;

    // Test Case 1: Correct password
    std::cout << "1. Correct password ('" << correct_password << "'): "
              << (verify_password(test_file_path, correct_password) ? "PASSED" : "FAILED") << std::endl;

    // Test Case 2: Incorrect password
    const std::string wrong_password = "WrongPassword";
    std::cout << "2. Incorrect password ('" << wrong_password << "'): "
              << (!verify_password(test_file_path, wrong_password) ? "PASSED" : "FAILED") << std::endl;
              
    // Test Case 3: Empty password
    std::cout << "3. Empty password (''): "
              << (!verify_password(test_file_path, "") ? "PASSED" : "FAILED") << std::endl;

    // Test Case 4: Password with a small difference
    const std::string diff_password = "PasswordForC++App2!";
    std::cout << "4. Slightly different password ('" << diff_password << "'): "
              << (!verify_password(test_file_path, diff_password) ? "PASSED" : "FAILED") << std::endl;

    // Test Case 5: Non-existent file
    std::cout << "5. Non-existent file: "
              << (!verify_password("nonexistentfile.txt", correct_password) ? "PASSED" : "FAILED") << std::endl;

    if (remove(test_file_path.c_str()) == 0) {
        std::cout << "Test file cleaned up." << std::endl;
    } else {
        std::cerr << "Error cleaning up test file." << std::endl;
    }

    return 0;
}