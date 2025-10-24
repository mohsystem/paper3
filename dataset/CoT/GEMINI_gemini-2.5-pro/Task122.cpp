/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against it. Example command:
 * g++ -o your_program_name Task122.cpp -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

class Task122 {
private:
    // In a real application, this would be a database.
    std::map<std::string, std::string> userDatabase;

    const int SALT_LENGTH = 16;
    const int KEY_LENGTH = 32; // 256 bits
    const int ITERATION_COUNT = 65536;

    // Helper to convert bytes to a hex string for storage
    std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (const auto& byte : bytes) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

    // Helper to convert a hex string back to bytes
    std::vector<unsigned char> hexToBytes(const std::string& hex) {
        std::vector<unsigned char> bytes;
        for (unsigned int i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

public:
    bool signUp(const std::string& username, const std::string& password) {
        if (userDatabase.count(username)) {
            std::cerr << "Error: User '" << username << "' already exists." << std::endl;
            return false;
        }

        // 1. Generate a random salt
        std::vector<unsigned char> salt(SALT_LENGTH);
        if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
            throw std::runtime_error("Error generating random salt.");
        }

        // 2. Generate the hash using PBKDF2
        std::vector<unsigned char> hash(KEY_LENGTH);
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(),
            password.length(),
            salt.data(),
            salt.size(),
            ITERATION_COUNT,
            EVP_sha256(),
            hash.size(),
            hash.data()
        );

        if (result != 1) {
            throw std::runtime_error("Error hashing password with PBKDF2.");
        }

        // 3. Store salt and hash as hex strings
        std::string storedPassword = bytesToHex(salt) + ":" + bytesToHex(hash);
        userDatabase[username] = storedPassword;
        return true;
    }

    bool verifyPassword(const std::string& username, const std::string& password) {
        if (!userDatabase.count(username)) {
            return false; // User not found
        }

        std::string storedData = userDatabase.at(username);
        size_t separatorPos = storedData.find(':');
        if (separatorPos == std::string::npos) {
            return false; // Invalid stored data format
        }

        // 1. Extract salt and hash from storage
        std::string saltHex = storedData.substr(0, separatorPos);
        std::string storedHashHex = storedData.substr(separatorPos + 1);
        
        std::vector<unsigned char> salt = hexToBytes(saltHex);
        std::vector<unsigned char> storedHash = hexToBytes(storedHashHex);

        // 2. Hash the provided password with the retrieved salt
        std::vector<unsigned char> providedHash(KEY_LENGTH);
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(),
            password.length(),
            salt.data(),
            salt.size(),
            ITERATION_COUNT,
            EVP_sha256(),
            providedHash.size(),
            providedHash.data()
        );

        if (result != 1) {
            return false; // Hashing failed
        }
        
        // 3. Compare hashes using a constant-time comparison function
        if (storedHash.size() != providedHash.size()) {
            return false; // Should never happen if KEY_LENGTH is constant
        }

        return CRYPTO_memcmp(storedHash.data(), providedHash.data(), storedHash.size()) == 0;
    }
};

int main() {
    Task122 manager;
    std::cout << "--- Test Cases ---" << std::endl;
    
    // Test Case 1: Successful signup
    std::cout << "\n[Test 1] Successful Signup" << std::endl;
    bool signup1 = manager.signUp("alice", "Password123!");
    std::cout << "Signup for 'alice' successful: " << std::boolalpha << signup1 << std::endl;
    
    // Test Case 2: Successful login
    std::cout << "\n[Test 2] Successful Login" << std::endl;
    bool login2 = manager.verifyPassword("alice", "Password123!");
    std::cout << "Login for 'alice' with correct password successful: " << std::boolalpha << login2 << std::endl;

    // Test Case 3: Failed login (wrong password)
    std::cout << "\n[Test 3] Failed Login (Wrong Password)" << std::endl;
    bool login3 = manager.verifyPassword("alice", "WrongPass!");
    std::cout << "Login for 'alice' with incorrect password successful: " << std::boolalpha << login3 << std::endl;

    // Test Case 4: Failed login (user does not exist)
    std::cout << "\n[Test 4] Failed Login (User Not Found)" << std::endl;
    bool login4 = manager.verifyPassword("bob", "Password123!");
    std::cout << "Login for 'bob' (non-existent user) successful: " << std::boolalpha << login4 << std::endl;

    // Test Case 5: Failed signup (user already exists)
    std::cout << "\n[Test 5] Failed Signup (User Exists)" << std::endl;
    manager.signUp("charlie", "MySecretPa$$");
    bool signup5 = manager.signUp("charlie", "AnotherPassword");
    std::cout << "Second signup for 'charlie' successful: " << std::boolalpha << signup5 << std::endl;

    return 0;
}