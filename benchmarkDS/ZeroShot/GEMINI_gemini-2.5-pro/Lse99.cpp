#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>

// --- IMPORTANT ---
// This code requires the OpenSSL library.
// To compile on Linux/macOS, use:
// g++ -std=c++17 your_file_name.cpp -o your_program -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>

// A simple in-memory map to simulate a database.
// In a real application, this would be a proper database.
struct UserData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

static std::map<std::string, UserData> userDatabase;

// Constants for hashing
const int SALT_BYTES = 16;
const int HASH_BYTES = 32; // 256 bits for SHA256
const int HASH_ITERATIONS = 260000; // Recommended for PBKDF2-SHA256

/**
 * Converts a vector of bytes to a hexadecimal string for display.
 */
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

/**
 * "Inserts" a new user into the database after hashing their password.
 * @param username The username.
 * @param password The plaintext password.
 */
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return;
    }
    
    if (userDatabase.count(username)) {
        std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
        return;
    }

    // 1. Generate a cryptographically secure random salt
    std::vector<unsigned char> salt(SALT_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        std::cerr << "Error: Failed to generate random salt." << std::endl;
        return;
    }

    // 2. Hash the password using PBKDF2
    std::vector<unsigned char> hashedPassword(HASH_BYTES);
    int success = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        HASH_ITERATIONS,
        EVP_sha256(),
        hashedPassword.size(),
        hashedPassword.data()
    );

    if (success != 1) {
        std::cerr << "Error: Failed to hash password." << std::endl;
        return;
    }

    // 3. Simulate inserting into the database
    userDatabase[username] = {salt, hashedPassword};
    std::cout << "User '" << username << "' registered successfully." << std::endl;
}

void printDatabaseState() {
    std::cout << "\n--- Simulated Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "'" << pair.first << "':\n"
                  << "  Salt:           " << to_hex(pair.second.salt) << "\n"
                  << "  Hashed Password: " << to_hex(pair.second.hashedPassword) << std::endl;
    }
}

int main() {
    // --- 5 Test Cases ---
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    registerUser("alice", "Password123!");
    registerUser("bob", "My$trongP@ssw0rd");
    registerUser("charlie", "Ch@rlieBrown");
    registerUser("diana", "Secr3t-P@ss");
    registerUser("eve", "password");

    printDatabaseState();
    
    return 0;
}