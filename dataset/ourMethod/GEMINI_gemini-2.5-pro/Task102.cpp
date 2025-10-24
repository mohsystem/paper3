#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>

// Note: This code requires linking with OpenSSL libraries, e.g., g++ ... -lssl -lcrypto

// Configuration for PBKDF2
const int ITERATIONS = 210000;
const int KEY_LENGTH = 32; // 256 bits
const int SALT_SIZE = 16;  // 128 bits

// In-memory map to simulate a user database
std::unordered_map<std::string, std::string> userDatabase;

// Helper function to convert bytes to a hex string
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Helper function to convert a hex string to bytes
std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    if (hex.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even number of characters.");
    }
    std::vector<unsigned char> bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string hash_password(const std::string& password) {
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }

    std::vector<unsigned char> hash(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt.data(), salt.size(),
        ITERATIONS,
        EVP_sha256(),
        hash.size(), hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("Failed to execute PBKDF2.");
    }

    return bytes_to_hex(salt) + ":" + bytes_to_hex(hash);
}

bool verify_password(const std::string& password, const std::string& stored_hash) {
    size_t colon_pos = stored_hash.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    try {
        std::string salt_hex = stored_hash.substr(0, colon_pos);
        std::string hash_hex = stored_hash.substr(colon_pos + 1);

        std::vector<unsigned char> salt = hex_to_bytes(salt_hex);
        std::vector<unsigned char> original_hash = hex_to_bytes(hash_hex);

        if (salt.size() != SALT_SIZE || original_hash.size() != KEY_LENGTH) {
            return false;
        }

        std::vector<unsigned char> test_hash(KEY_LENGTH);
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            ITERATIONS,
            EVP_sha256(),
            test_hash.size(), test_hash.data()
        );

        if (result != 1) {
            return false;
        }

        // Constant-time comparison
        return CRYPTO_memcmp(original_hash.data(), test_hash.data(), KEY_LENGTH) == 0;
    } catch (const std::exception& e) {
        std::cerr << "Verification error: " << e.what() << std::endl;
        return false;
    }
}

bool update_user_password(const std::string& username, const std::string& new_password, 
                          std::unordered_map<std::string, std::string>& database) {
    if (username.empty() || username.find_first_not_of(' ') == std::string::npos) {
        std::cout << "Update failed: Username cannot be empty." << std::endl;
        return false;
    }
    if (new_password.length() < 12) {
        std::cout << "Update failed: Password must be at least 12 characters long." << std::endl;
        return false;
    }
    
    try {
        std::string new_hashed_password = hash_password(new_password);
        database[username] = new_hashed_password;
        std::cout << "Password for user '" << username << "' has been updated successfully." << std::endl;
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to hash the new password for user '" << username << "': " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Create a new user with a strong password
    std::cout << "\n[Test Case 1: Create new user 'alice']" << std::endl;
    update_user_password("alice", "Str0ngP@ssw0rd123!", userDatabase);
    std::cout << "Current hash for alice: " << userDatabase["alice"] << std::endl;

    // Test Case 2: Verify the correct password
    std::cout << "\n[Test Case 2: Verify correct password for 'alice']" << std::endl;
    bool is_verified = verify_password("Str0ngP@ssw0rd123!", userDatabase["alice"]);
    std::cout << "Verification result: " << (is_verified ? "Success" : "Failure") << std::endl;

    // Test Case 3: Verify an incorrect password
    std::cout << "\n[Test Case 3: Verify incorrect password for 'alice']" << std::endl;
    is_verified = verify_password("WrongPassword!", userDatabase["alice"]);
    std::cout << "Verification result: " << (is_verified ? "Success" : "Failure") << std::endl;

    // Test Case 4: Update the password for an existing user
    std::cout << "\n[Test Case 4: Update password for 'alice']" << std::endl;
    update_user_password("alice", "EvenM0reSecur3P@ss!", userDatabase);
    std::cout << "Current hash for alice: " << userDatabase["alice"] << std::endl;

    // Test Case 5: Verify the new password
    std::cout << "\n[Test Case 5: Verify the new password for 'alice']" << std::endl;
    is_verified = verify_password("EvenM0reSecur3P@ss!", userDatabase["alice"]);
    std::cout << "Verification result: " << (is_verified ? "Success" : "Failure") << std::endl;
    
    std::cout << "\n--- Additional Tests ---" << std::endl;
    // Test Case 6: Attempt to update password with a weak password
    std::cout << "\n[Test Case 6: Attempt weak password for 'bob']" << std::endl;
    update_user_password("bob", "weak", userDatabase);

    // Test Case 7: Attempt to update password with an empty username
    std::cout << "\n[Test Case 7: Attempt update with empty username]" << std::endl;
    update_user_password(" ", "ValidPassword12345", userDatabase);

    return 0;
}