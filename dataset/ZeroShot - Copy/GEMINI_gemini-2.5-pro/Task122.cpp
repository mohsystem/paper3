/*
 * NOTE: This code requires the OpenSSL library.
 * To compile, you need to link against OpenSSL libraries.
 * Example compilation command:
 * g++ -std=c++17 -o Task122 Task122.cpp -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/evp.h>

class Task122 {
private:
    static const int SALT_SIZE = 16;    // 128 bits
    static const int HASH_SIZE = 32;    // 256 bits
    static const int ITERATIONS = 65536;

    // Helper to convert byte array to hex string
    static std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (const auto& byte : bytes) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

    // Helper to convert hex string to byte array
    static std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
        if (hex.length() % 2 != 0) {
            throw std::invalid_argument("Hex string must have an even number of characters.");
        }
        std::vector<unsigned char> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

public:
    /**
     * Stores a user password securely by generating a salt and hashing the password using PBKDF2.
     * @param password The plaintext password to store.
     * @return A string containing the salt and hash, separated by a colon, in hex format.
     */
    static std::string signup(const std::string& password) {
        std::vector<unsigned char> salt(SALT_SIZE);
        if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
            throw std::runtime_error("Error generating random salt.");
        }

        std::vector<unsigned char> hash(HASH_SIZE);
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(),
            password.length(),
            salt.data(),
            salt.size(),
            ITERATIONS,
            EVP_sha256(),
            hash.size(),
            hash.data()
        );

        if (result != 1) {
            throw std::runtime_error("Error during password hashing.");
        }

        return bytes_to_hex(salt) + ":" + bytes_to_hex(hash);
    }

    /**
     * Verifies a plaintext password against a stored salted hash.
     * @param password The plaintext password to verify.
     * @param storedPassword The stored string containing the salt and hash.
     * @return true if the password is correct, false otherwise.
     */
    static bool verifyPassword(const std::string& password, const std::string& storedPassword) {
        try {
            size_t separator_pos = storedPassword.find(':');
            if (separator_pos == std::string::npos) {
                return false; // Invalid format
            }

            std::string salt_hex = storedPassword.substr(0, separator_pos);
            std::string stored_hash_hex = storedPassword.substr(separator_pos + 1);

            std::vector<unsigned char> salt = hex_to_bytes(salt_hex);
            std::vector<unsigned char> stored_hash = hex_to_bytes(stored_hash_hex);

            std::vector<unsigned char> test_hash(HASH_SIZE);
            int result = PKCS5_PBKDF2_HMAC(
                password.c_str(),
                password.length(),
                salt.data(),
                salt.size(),
                ITERATIONS,
                EVP_sha256(),
                test_hash.size(),
                test_hash.data()
            );

            if (result != 1) {
                return false; // Hashing failed
            }
            
            // Constant-time comparison
            return CRYPTO_memcmp(stored_hash.data(), test_hash.data(), HASH_SIZE) == 0;
        } catch (const std::exception&) {
            // Catches errors from hex decoding
            return false;
        }
    }
};

int main() {
    std::cout << "--- C++ Password Hashing Tests ---" << std::endl;

    // Test Case 1: Sign up a user and store the password hash
    std::cout << "\n[Test Case 1: Signup with 'password123']" << std::endl;
    std::string user1_password = "password123";
    std::string stored_user1 = Task122::signup(user1_password);
    std::cout << "Stored format for '" << user1_password << "': " << stored_user1 << std::endl;

    // Test Case 2: Verify with the correct password
    std::cout << "\n[Test Case 2: Verify with correct password 'password123']" << std::endl;
    bool is_correct = Task122::verifyPassword(user1_password, stored_user1);
    std::cout << "Verification successful: " << std::boolalpha << is_correct << std::endl;
    if (!is_correct) std::cout << "TEST FAILED!" << std::endl;

    // Test Case 3: Verify with an incorrect password
    std::cout << "\n[Test Case 3: Verify with incorrect password 'wrongpassword']" << std::endl;
    bool is_incorrect = Task122::verifyPassword("wrongpassword", stored_user1);
    std::cout << "Verification successful: " << std::boolalpha << is_incorrect << std::endl;
    if (is_incorrect) std::cout << "TEST FAILED!" << std::endl;

    // Test Case 4: Sign up another user with a different password
    std::cout << "\n[Test Case 4: Signup with a different password 'anotherPassword']" << std::endl;
    std::string user2_password = "anotherPassword";
    std::string stored_user2 = Task122::signup(user2_password);
    std::cout << "Stored format for '" << user2_password << "': " << stored_user2 << std::endl;
    std::cout << "Hashes for different passwords are different: " << std::boolalpha << (stored_user1 != stored_user2) << std::endl;

    // Test Case 5: Sign up with the same password again, salt should be different
    std::cout << "\n[Test Case 5: Signup with 'password123' again]" << std::endl;
    std::string stored_user1_again = Task122::signup(user1_password);
    std::cout << "Second stored format for '" << user1_password << "': " << stored_user1_again << std::endl;
    std::cout << "Hashes for the same password are different due to salt: " << std::boolalpha << (stored_user1 != stored_user1_again) << std::endl;

    return 0;
}