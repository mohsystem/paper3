/*
To compile this code, you need the OpenSSL library installed.
Compile using g++:
g++ your_file_name.cpp -o Lse93 -lcrypto

This code uses PBKDF2 with SHA256 for password hashing, which is a secure standard.
*/
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

// A simple in-memory map to simulate a database.
static std::map<std::string, std::string> userDatabase;

// Constants for PBKDF2
const int SALT_LEN = 16;
const int KEY_LEN = 32; // 256 bits for SHA256
const int ITERATIONS = 100000; // A reasonable number of iterations

// Helper function to convert binary bytes to a hex string
std::string to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

/**
 * Hashes a password and stores the user record in a simulated database.
 * The stored format is "salt_hex$hash_hex".
 *
 * @param username The username to register.
 * @param password The plain-text password to hash.
 */
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return;
    }

    if (userDatabase.count(username)) {
        std::cerr << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return;
    }

    // 1. Generate a random salt
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        std::cerr << "Error generating salt." << std::endl;
        return;
    }

    // 2. Hash the password using PBKDF2
    unsigned char key[KEY_LEN];
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(), 
        salt, 
        sizeof(salt), 
        ITERATIONS, 
        EVP_sha256(), 
        sizeof(key), 
        key
    );

    if (result != 1) {
        std::cerr << "Error in PKCS5_PBKDF2_HMAC." << std::endl;
        return;
    }

    // 3. Store the salt and hashed password
    std::string salt_hex = to_hex(salt, sizeof(salt));
    std::string key_hex = to_hex(key, sizeof(key));
    
    // Store in a format that's easy to parse later for verification
    userDatabase[username] = salt_hex + "$" + key_hex;

    std::cout << "User '" << username << "' registered successfully." << std::endl;
}

int main() {
    std::cout << "--- Running 5 Registration Test Cases ---" << std::endl;

    // Test Case 1: Standard registration
    registerUser("alice", "Password123!");

    // Test Case 2: Another standard registration
    registerUser("bob", "my$ecretP@ss");

    // Test Case 3: Registration with a different password
    registerUser("charlie", "Ch@rlieR0cks");

    // Test Case 4: Attempting to register a user that already exists
    registerUser("alice", "another_password");

    // Test Case 5: Registration with an empty password (should fail)
    registerUser("david", "");
    
    std::cout << "\n--- Simulated Database Contents ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << ", Stored Hash: " << pair.second << std::endl;
    }

    return 0;
}