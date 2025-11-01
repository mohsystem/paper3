/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * g++ -std=c++17 your_file_name.cpp -o Lse92 -lssl -lcrypto
 *
 * This code demonstrates secure password hashing using PBKDF2 from OpenSSL.
 */
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <map>

#include <openssl/evp.h>
#include <openssl/rand.h>

// In a real application, this would be a secure database.
std::map<std::string, std::pair<std::string, std::string>> userDatabase;

// Helper function to convert a byte array to a hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

/**
 * Registers a new user by securely hashing their password.
 * @param username The username to register.
 * @param password The user's password.
 * @return True if registration is successful, false otherwise.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Registration failed: Username and password cannot be empty." << std::endl;
        return false;
    }
    
    // In a real app, check if the username already exists.
    if (userDatabase.count(username)) {
        std::cerr << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    // Generate a random salt (16 bytes is a good size)
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        std::cerr << "Error generating salt." << std::endl;
        return false;
    }

    // Use PBKDF2 for hashing
    unsigned char hashedPassword[32]; // For SHA-256, 32 bytes output
    int iterations = 100000;
    
    if (PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(),
        salt, 
        sizeof(salt),
        iterations,
        EVP_sha256(),
        sizeof(hashedPassword),
        hashedPassword
    ) == 0) {
        std::cerr << "Error in PKCS5_PBKDF2_HMAC." << std::endl;
        return false;
    }
    
    std::string saltHex = bytes_to_hex(salt, sizeof(salt));
    std::string hashedPasswordHex = bytes_to_hex(hashedPassword, sizeof(hashedPassword));

    // Store the user record (simulated)
    userDatabase[username] = {saltHex, hashedPasswordHex};

    std::cout << "User '" << username << "' registered successfully." << std::endl;
    std::cout << "  - Salt (Hex): " << saltHex << std::endl;
    std::cout << "  - Hashed Password (Hex): " << hashedPasswordHex << std::endl;
    std::cout << "------------------------------------" << std::endl;

    return true;
}

int main() {
    std::cout << "--- C++ Registration Test Cases ---" << std::endl;
    registerUser("alice", "Password123!");
    registerUser("bob", "my$ecretP@ss");
    registerUser("charlie", "aVeryLongAndComplexPassword");
    registerUser("dave", "short");
    registerUser("", "some_password"); // Test empty username
    std::cout << "Total users in DB: " << userDatabase.size() << std::endl;

    return 0;
}