#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp
#include <iomanip>
#include <sstream>

// To compile: g++ -Wall -std=c++17 your_file_name.cpp -o your_program -lssl -lcrypto

// Constants for password hashing
constexpr int SALT_BYTES = 16;
constexpr int KEY_BYTES = 32; // 256 bits
constexpr int ITERATIONS = 600000;

struct UserEntry {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

// In-memory user database simulation
static std::map<std::string, UserEntry> user_database;

// Helper to print byte vectors as hex for demonstration
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_BYTES);
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
        throw std::runtime_error("Failed to hash password with PBKDF2");
    }
    return hash;
}

bool register_user(const std::string& username, std::string password) {
    if (username.empty()) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return false;
    }
    if (password.length() < 8) {
        std::cerr << "Error: Password must be at least 8 characters long." << std::endl;
        return false;
    }
    if (user_database.count(username)) {
        std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    // Generate a cryptographically secure random salt
    std::vector<unsigned char> salt(SALT_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        std::cerr << "Error: Failed to generate random salt." << std::endl;
        return false;
    }

    try {
        std::vector<unsigned char> hashed_password = hash_password(password, salt);
        user_database[username] = {salt, hashed_password};
        std::cout << "User '" << username << "' registered successfully." << std::endl;
        
        // Securely clear password from memory after use
        OPENSSL_cleanse(&password[0], password.size());
        
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "Registration failed: " << e.what() << std::endl;
        return false;
    }
}

bool verify_password(const std::string& username, std::string password) {
    auto it = user_database.find(username);
    if (it == user_database.end()) {
        return false; // User not found
    }

    const UserEntry& entry = it->second;

    try {
        std::vector<unsigned char> hash_to_verify = hash_password(password, entry.salt);
        
        // Securely clear password from memory after use
        OPENSSL_cleanse(&password[0], password.size());

        // Constant-time comparison to prevent timing attacks
        return (entry.hash.size() == hash_to_verify.size()) && 
               (CRYPTO_memcmp(entry.hash.data(), hash_to_verify.data(), entry.hash.size()) == 0);
    } catch (const std::runtime_error& e) {
        std::cerr << "Verification failed: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Successful registration and login
    std::cout << "\n--- Test Case 1: Successful Registration & Login ---" << std::endl;
    register_user("alice", "Password123!");
    std::cout << "Verifying 'alice' with correct password: " << (verify_password("alice", "Password123!") ? "SUCCESS" : "FAILURE") << std::endl;
    auto alice_entry = user_database.find("alice");
    if(alice_entry != user_database.end()) {
        std::cout << "Stored Salt (Hex): " << bytes_to_hex(alice_entry->second.salt) << std::endl;
        std::cout << "Stored Hash (Hex): " << bytes_to_hex(alice_entry->second.hash) << std::endl;
    }

    // Test Case 2: Failed login attempt with wrong password
    std::cout << "\n--- Test Case 2: Failed Login ---" << std::endl;
    std::cout << "Verifying 'alice' with incorrect password: " << (verify_password("alice", "WrongPassword!") ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3: Attempt to register a user that already exists
    std::cout << "\n--- Test Case 3: Duplicate Registration ---" << std::endl;
    register_user("alice", "AnotherPassword456!");

    // Test Case 4: Attempt to register with a weak password
    std::cout << "\n--- Test Case 4: Weak Password Registration ---" << std::endl;
    register_user("bob", "pass");

    // Test Case 5: Successful registration of a second user
    std::cout << "\n--- Test Case 5: Successful Registration of Another User ---" << std::endl;
    register_user("charlie", "S3cureP@ssw0rdF0rCharlie");
    std::cout << "Verifying 'charlie' with correct password: " << (verify_password("charlie", "S3cureP@ssw0rdF0rCharlie") ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Verifying 'charlie' with incorrect password: " << (verify_password("charlie", "wrongpass") ? "SUCCESS" : "FAILURE") << std::endl;

    return 0;
}