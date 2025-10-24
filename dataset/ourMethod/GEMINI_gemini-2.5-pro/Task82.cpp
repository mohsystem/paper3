#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>

const int ITERATIONS = 600000;
const int KEY_LENGTH = 32; // 32 bytes = 256 bits
const int SALT_LENGTH = 16; // 16 bytes = 128 bits

/**
 * Converts a byte vector to a hex string.
 *
 * @param bytes The vector of bytes.
 * @return A hex-encoded string.
 */
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Generates a cryptographically secure random salt.
 *
 * @return A vector containing 16 bytes of salt.
 */
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }
    return salt;
}

/**
 * Computes the hash of a password combined with a salt using PBKDF2 with HMAC-SHA256.
 *
 * @param password The password to hash.
 * @param salt The salt to use.
 * @return The resulting hash as a hex-encoded string.
 */
std::string hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    if (password.empty() || salt.empty()) {
        throw std::invalid_argument("Password and salt must not be empty.");
    }
    std::vector<unsigned char> hash(KEY_LENGTH);
    int success = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATIONS,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );

    if (success != 1) {
        throw std::runtime_error("PBKDF2 HMAC failed.");
    }

    return bytes_to_hex(hash);
}

int main() {
    const std::vector<std::string> test_passwords = {
        "P@ssword123!",
        "correct horse battery staple",
        "Tr0ub4dor&3",
        "123456",
        "super-secret-password"
    };

    std::cout << "Running 5 test cases for password hashing..." << std::endl;

    for (const auto& password : test_passwords) {
        try {
            // In a real application, never log or print the plaintext password.
            // This is for demonstration purposes only.
            std::cout << "\n--- Test Case ---" << std::endl;
            std::cout << "Password (for demo only): " << password << std::endl;

            std::vector<unsigned char> salt = generate_salt();
            std::cout << "Generated Salt (hex): " << bytes_to_hex(salt) << std::endl;

            std::string hashed_password = hash_password(password, salt);
            std::cout << "Hashed Password (hex): " << hashed_password << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;
        }
    }

    return 0;
}