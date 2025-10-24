/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you must link against OpenSSL libraries.
 * Example compilation command on Linux/macOS:
 * g++ -std=c++17 your_file_name.cpp -o your_program_name -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// In-memory map to simulate a database
static std::map<std::string, std::string> userDatabase;

const int SALT_SIZE = 16;
const int HASH_SIZE = 64; // For SHA-512
const int ITERATIONS = 65536;

/**
 * Converts a byte array to a hex string.
 */
std::string bytes_to_hex(const unsigned char* data, int len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

/**
 * Hashes a password using PBKDF2-HMAC-SHA512.
 * Returns a string in format "salt_hex:hash_hex"
 */
std::string hash_password(const std::string& password) {
    unsigned char salt[SALT_SIZE];
    if (!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Error generating salt");
    }

    unsigned char hash[HASH_SIZE];
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha512(),
        sizeof(hash),
        hash
    );

    if (result != 1) {
        throw std::runtime_error("Error in PKCS5_PBKDF2_HMAC");
    }

    return bytes_to_hex(salt, sizeof(salt)) + ":" + bytes_to_hex(hash, sizeof(hash));
}


/**
 * Accepts username and a new password, hashes the password, and updates the database.
 */
bool updateUserPassword(const std::string& username, const std::string& newPassword) {
    if (username.empty() || newPassword.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return false;
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cerr << "Error: User '" << username << "' not found." << std::endl;
        return false;
    }

    try {
        std::string stored_value = hash_password(newPassword);
        userDatabase[username] = stored_value;
        std::cout << "Password for user '" << username << "' has been updated successfully." << std::endl;
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to hash password for user '" << username << "': " << e.what() << std::endl;
        return false;
    }
}

void printDatabase() {
    for (const auto& pair : userDatabase) {
        std::cout << "User: " << pair.first << ", Stored: " << pair.second << std::endl;
    }
}

int main() {
    // Initialize the 'database'
    userDatabase["alice"] = "initial_placeholder_hash";
    userDatabase["bob"] = "initial_placeholder_hash";
    userDatabase["charlie"] = "initial_placeholder_hash";
    userDatabase["david"] = "initial_placeholder_hash";
    userDatabase["eve"] = "initial_placeholder_hash";

    std::cout << "--- Initial Database State ---" << std::endl;
    printDatabase();
    
    std::cout << "\n--- Running 5 Test Cases for Password Update ---" << std::endl;

    // Test Case 1: Update alice's password
    updateUserPassword("alice", "P@ssword123!");

    // Test Case 2: Update bob's password
    updateUserPassword("bob", "SecurePwd!#$987");

    // Test Case 3: Update a non-existent user
    updateUserPassword("frank", "somePassword");

    // Test Case 4: Update charlie's password
    updateUserPassword("charlie", "another-strong-password-4-charlie");

    // Test Case 5: Update with an empty password
    updateUserPassword("david", "");

    std::cout << "\n--- Final Database State ---" << std::endl;
    printDatabase();

    return 0;
}