#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// IMPORTANT: This code requires the OpenSSL library.
// To compile on Linux/macOS: g++ your_file_name.cpp -o your_program_name -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

// Struct to hold user data in our simulated database
struct UserData {
    std::vector<unsigned char> salt;
    std::string hashedPassword;
};

// Simulate a database table: username -> UserData
std::map<std::string, UserData> database;

// Helper to convert byte vector to hex string
std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Generates a salt, hashes the password, and "inserts" into the database.
 * @param username The username to register.
 * @param password The plain-text password.
 * @return A copy of the stored UserData. Throws runtime_error on failure.
 */
UserData registerUser(const std::string& username, const std::string& password) {
    // 1. Generate a random salt
    const int SALT_SIZE = 16;
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }

    // 2. Combine salt and password bytes
    std::vector<unsigned char> toHash;
    toHash.reserve(salt.size() + password.length());
    toHash.insert(toHash.end(), salt.begin(), salt.end());
    toHash.insert(toHash.end(), password.begin(), password.end());

    // 3. Hash the combined bytes using SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(toHash.data(), toHash.size(), hash);

    std::vector<unsigned char> hashVec(hash, hash + SHA256_DIGEST_LENGTH);
    std::string hashedPasswordHex = bytesToHex(hashVec);

    // 4. Store username, salt, and hashed password
    UserData newUser = {salt, hashedPasswordHex};
    database[username] = newUser;

    std::cout << "User '" << username << "' registered successfully." << std::endl;
    std::cout << "  - Salt (Hex): " << bytesToHex(salt) << std::endl;
    std::cout << "  - Hashed Password (Hex): " << hashedPasswordHex << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    return newUser;
}

// Using a class to meet the naming requirement
class Lse94 {
public:
    static void run() {
        std::cout << "--- Running 5 Test Cases for User Registration ---" << std::endl << std::endl;
        
        try {
            // Test Case 1
            registerUser("alice", "password123");

            // Test Case 2
            registerUser("bob", "my$ecretP@ss");

            // Test Case 3
            registerUser("charlie", "CharlieBrown");

            // Test Case 4
            registerUser("dave", "davespassword");

            // Test Case 5: Registering a user with the same password as alice
            registerUser("eve", "password123");

        } catch (const std::runtime_error& e) {
            std::cerr << "A critical error occurred: " << e.what() << std::endl;
        }

        std::cout << std::endl << "--- Final Database State ---" << std::endl;
        for (const auto& pair : database) {
            std::cout << "Username: " << pair.first 
                      << ", HashedPassword: " << pair.second.hashedPassword 
                      << std::endl;
        }
    }
};

int main() {
    Lse94::run();
    return 0;
}