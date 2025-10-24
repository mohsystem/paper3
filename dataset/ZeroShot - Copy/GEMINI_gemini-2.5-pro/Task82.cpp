#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
// Requires OpenSSL library. Compile with: g++ <filename>.cpp -o <output_name> -l crypto
#include <openssl/evp.h>

const int ITERATIONS = 65536;
const int KEY_LENGTH = 32; // bytes, equivalent to 256 bits

/**
 * Computes the hash of a password combined with a salt using PBKDF2-HMAC-SHA256.
 *
 * @param password The password to hash.
 * @param salt The salt to use. Must be a cryptographically random sequence of bytes.
 * @return A vector of unsigned char containing the hash.
 */
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);
    
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.length()),
        salt.data(),
        static_cast<int>(salt.size()),
        ITERATIONS,
        EVP_sha256(),
        static_cast<int>(hash.size()),
        hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("Error generating PBKDF2 hash.");
    }
    
    return hash;
}

/**
 * Helper function to print a vector of unsigned chars as a hexadecimal string.
 */
void printHex(const std::string& label, const std::vector<unsigned char>& data) {
    std::cout << label;
    std::cout << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        std::cout << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

// Helper to create a vector from a string literal
std::vector<unsigned char> stringToVec(const std::string& s) {
    return std::vector<unsigned char>(s.begin(), s.end());
}

int main() {
    std::vector<std::string> passwords = {
        "password123",
        "P@$$w0rd!",
        "correct horse battery staple",
        "123",
        ""
    };

    std::vector<std::vector<unsigned char>> salts = {
        stringToVec("salt1234"),
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
        stringToVec("another-random-salt"),
        {}, // Empty salt
        stringToVec("a_very_long_salt_for_testing_purposes_with_an_empty_password")
    };

    std::cout << "C++ Password Hashing Tests:" << std::endl;
    for (size_t i = 0; i < passwords.size(); ++i) {
        const std::string& password = passwords[i];
        const std::vector<unsigned char>& salt = salts[i];
        
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Password: \"" << password << "\"" << std::endl;
        printHex("Salt (hex): ", salt);
        
        try {
            std::vector<unsigned char> hash = hashPassword(password, salt);
            printHex("Hashed Password (hex): ", hash);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}