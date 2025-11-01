#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/evp.h>

// This code requires the OpenSSL library.
// To compile on Linux/macOS: g++ -o your_program_name your_source_file.cpp -lssl -lcrypto
// You must have OpenSSL development libraries installed (e.g., `libssl-dev` on Debian/Ubuntu).

// Hashing parameters
const int SALT_LENGTH = 16;
const int ITERATIONS = 65536;
const int KEY_LENGTH = 32; // Corresponds to 256 bits for SHA-256

/**
 * Generates a cryptographically secure random salt.
 * @return A vector of unsigned characters containing the salt.
 */
std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }
    return salt;
}

/**
 * Hashes the password using the PBKDF2 algorithm with HMAC-SHA256.
 * @param password The password to hash.
 * @param salt The salt to use.
 * @return A vector of unsigned characters containing the hashed password.
 */
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty.");
    }
    std::vector<unsigned char> key(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        key.data()
    );
    if (result != 1) {
        throw std::runtime_error("Error in PKCS5_PBKDF2_HMAC.");
    }
    return key;
}

/**
 * Converts a byte vector to a hexadecimal string.
 * @param bytes The vector of bytes to convert.
 * @return The resulting hexadecimal string.
 */
std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Simulates user registration by generating salt, hashing password, and preparing for storage.
 * @param username The username (part of a typical registration).
 * @param password The plain-text password.
 * @return A string containing the salt and hash, concatenated with a separator.
 */
std::string registerUser(const std::string& username, const std::string& password) {
    // 1. Generate salt
    std::vector<unsigned char> salt = generateSalt();

    // 2. Hash the password
    std::vector<unsigned char> hashedPassword = hashPassword(password, salt);

    // 3. Convert to hex strings and concatenate for storage
    std::string saltHex = bytesToHex(salt);
    std::string hashHex = bytesToHex(hashedPassword);
    
    return saltHex + ":" + hashHex;
}

class Lse96 {
public:
    static void main() {
        std::cout << "--- Running C++ Registration Test Cases ---" << std::endl;

        try {
            // Test Case 1
            std::string storedCredentials1 = registerUser("user1", "Password123!");
            std::cout << "User: user1, Stored: " << storedCredentials1 << std::endl;

            // Test Case 2
            std::string storedCredentials2 = registerUser("admin_test", "S3cureP@ssw0rd");
            std::cout << "User: admin_test, Stored: " << storedCredentials2 << std::endl;

            // Test Case 3
            std::string storedCredentials3 = registerUser("jane.doe", "another-password-4-jane");
            std::cout << "User: jane.doe, Stored: " << storedCredentials3 << std::endl;

            // Test Case 4: Simple password
            std::string storedCredentials4 = registerUser("test_user", "password");
            std::cout << "User: test_user, Stored: " << storedCredentials4 << std::endl;

            // Test Case 5: Long password with many special characters
            std::string storedCredentials5 = registerUser("poweruser", "L0ngP@$$w0rd_With_Many_Ch@r$!#%^&*()");
            std::cout << "User: poweruser, Stored: " << storedCredentials5 << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "An error occurred: " << e.what() << std::endl;
        }
    }
};

int main() {
    Lse96::main();
    return 0;
}