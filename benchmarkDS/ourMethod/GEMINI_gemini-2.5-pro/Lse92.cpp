#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

const int ITERATIONS = 210000;
const int SALT_SIZE = 16; // bytes
const int KEY_LENGTH = 32; // bytes, 256 bits

// Helper function to convert bytes to a hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

/**
 * @brief Hashes a password using PBKDF2 with HMAC-SHA256.
 *
 * @param username The username (for context, not used in hash).
 * @param password The password to hash.
 * @return A string containing the salt and hash in hex format, separated by a colon.
 */
std::string registerUser(const std::string& username, const std::string& password) {
    // Rule #7: Use a strong, cryptographically secure random number generator.
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Error generating random salt: " + 
            std::string(ERR_error_string(ERR_get_error(), nullptr)));
    }

    // Rule #5: Use PBKDF2-HMAC-SHA-256.
    unsigned char hash[KEY_LENGTH];
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash),
        hash
    );

    if (result != 1) {
        throw std::runtime_error("Error in PBKDF2 HMAC: " +
            std::string(ERR_error_string(ERR_get_error(), nullptr)));
    }

    // Rule #6: Store the salt with the hash.
    // Here we convert to hex for storage.
    std::string salt_hex = bytes_to_hex(salt, sizeof(salt));
    std::string hash_hex = bytes_to_hex(hash, sizeof(hash));

    return salt_hex + ":" + hash_hex;
}

int main() {
    std::cout << "Running registration simulation..." << std::endl;

    // Test cases
    const char* test_users[][2] = {
        {"alice", "Password123!"},
        {"bob", "my$ecretP@ssw0rd"},
        {"charlie", "an0th3r_sTr0ng_p@ss"},
        {"david", "w3ak"},
        {"eve", "Eve's_Password_is_very_long_and_secure_hopefully"}
    };

    for (const auto& user : test_users) {
        std::string username = user[0];
        std::string password = user[1];
        try {
            std::string hashedPassword = registerUser(username, password);
            std::cout << "Username: " << username << std::endl;
            std::cout << "Password: [REDACTED]" << std::endl;
            std::cout << "Stored Hash: " << hashedPassword << std::endl;
            std::cout << "---" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to register user " << username << ": " << e.what() << std::endl;
        }
    }

    return 0;
}