// This code requires the OpenSSL library.
// Compile with: g++ your_file.cpp -o your_app -lssl -lcrypto
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>

// OpenSSL headers for hashing and random number generation
#include <openssl/evp.h>
#include <openssl/rand.h>

// Simulated in-memory database
static std::map<std::string, std::string> database;

// Constants for PBKDF2-HMAC-SHA256
const int SALT_BYTES = 16;
const int HASH_BYTES = 32; // 256 bits
const int ITERATIONS = 100000;

// Helper function to convert a byte vector to a hex string
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Registers a user by securely hashing the password and storing it.
 * @param username The username to register.
 * @param password The plaintext password.
 * @return True if registration is successful, false otherwise.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return false;
    }
    if (database.count(username)) {
        std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    // 1. Generate a cryptographically secure random salt
    std::vector<unsigned char> salt(SALT_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        std::cerr << "Critical Error: Failed to generate random salt." << std::endl;
        return false;
    }

    // 2. Hash the password using PBKDF2
    std::vector<unsigned char> hash(HASH_BYTES);
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
        std::cerr << "Critical Error: Failed to hash password." << std::endl;
        return false;
    }

    // 3. Store salt and hash together as hex strings separated by a colon
    std::string stored_value = to_hex(salt) + ":" + to_hex(hash);
    database[username] = stored_value;
    
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

int main() {
    std::cout << "--- C++ Registration Test ---" << std::endl;

    // Test Case 1: Successful registration
    registerUser("alice", "Password123!");
    
    // Test Case 2: Successful registration
    registerUser("bob", "SecurePass@2023");
    
    // Test Case 3: Attempt with an empty password
    registerUser("charlie", "");
    
    // Test Case 4: Attempt to register a duplicate username
    registerUser("alice", "AnotherPassword");
    
    // Test Case 5: Successful registration with complex password
    registerUser("frank", "fr@nk_p@$$w0rd");

    std::cout << "\n--- Simulated Database Contents ---" << std::endl;
    for (const auto& pair : database) {
        std::cout << "Username: " << pair.first << ", Stored Value: " << pair.second << std::endl;
    }
    std::cout << "---------------------------------" << std::endl << std::endl;

    return 0;
}