/*
 * NOTE: This code requires the OpenSSL library.
 * To compile and run:
 * 1. Ensure you have OpenSSL installed (e.g., `sudo apt-get install libssl-dev` on Debian/Ubuntu).
 * 2. Compile with: g++ -o task102_cpp your_file_name.cpp -lssl -lcrypto
 * 3. Run with: ./task102_cpp
 */
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

// --- Hashing Configuration ---
const int SALT_SIZE_BYTES = 16;
const int HASH_SIZE_BYTES = 32;
const int ITERATION_COUNT = 260000; // Recommended for PBKDF2-HMAC-SHA256

// UserData struct to hold salt and hash
struct UserData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

// In-memory map to simulate a user database
static std::unordered_map<std::string, UserData> userDatabase;

// Helper function to print byte vectors as hex strings for display
std::string to_hex_string(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

/**
 * Hashes a password using PBKDF2 with a given salt.
 *
 * @param password The plain-text password.
 * @param salt The salt to use.
 * @return A vector containing the password hash. Returns an empty vector on failure.
 */
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(HASH_SIZE_BYTES);
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
        // Hashing failed
        return {};
    }
    return hash;
}

/**
 * Updates a user's password in the simulated database.
 * This function generates a new salt, hashes the new password, and stores them.
 *
 * @param username The username of the user to update.
 * @param newPassword The new plain-text password.
 * @return true if the update was successful, false otherwise.
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

    // Generate a new random salt
    std::vector<unsigned char> newSalt(SALT_SIZE_BYTES);
    if (RAND_bytes(newSalt.data(), newSalt.size()) != 1) {
        std::cerr << "Critical error: Failed to generate random salt." << std::endl;
        return false;
    }

    // Hash the new password with the new salt
    std::vector<unsigned char> newHash = hashPassword(newPassword, newSalt);
    if (newHash.empty()) {
        std::cerr << "Critical error: Password hashing failed." << std::endl;
        return false;
    }

    // Update the user's data in the "database"
    it->second.salt = newSalt;
    it->second.hash = newHash;

    std::cout << "Successfully updated password for user '" << username << "'." << std::endl;
    return true;
}

// Helper to set up an initial user for testing
void setupInitialUser(const std::string& username, const std::string& password) {
    std::vector<unsigned char> salt(SALT_SIZE_BYTES);
    RAND_bytes(salt.data(), salt.size());
    std::vector<unsigned char> hash = hashPassword(password, salt);
    if (!hash.empty()) {
        userDatabase[username] = {salt, hash};
    }
}

void printDbState() {
    std::cout << "--- Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "  '" << pair.first << "':\n"
                  << "    Salt: " << to_hex_string(pair.second.salt) << "\n"
                  << "    Hash: " << to_hex_string(pair.second.hash) << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

int main() {
    // --- Setup: Pre-populate the database ---
    setupInitialUser("user1", "oldPassword123");
    setupInitialUser("user2", "anotherOldPwd");
    printDbState();

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Update password for an existing user
    std::cout << "\n[Test Case 1]: Update password for existing user 'user1'" << std::endl;
    updateUserPassword("user1", "newStrongPassword!@#");
    printDbState();

    // Test Case 2: Attempt to update password for a non-existent user
    std::cout << "\n[Test Case 2]: Attempt to update password for non-existent user 'unknownUser'" << std::endl;
    updateUserPassword("unknownUser", "somePassword");

    // Test Case 3: Attempt to update with an empty password
    std::cout << "\n[Test Case 3]: Attempt to update 'user2' with an empty password" << std::endl;
    updateUserPassword("user2", "");
    printDbState();
    
    // Test Case 4: Update password for the second user
    std::cout << "\n[Test Case 4]: Update password for existing user 'user2'" << std::endl;
    updateUserPassword("user2", "updatedP@ssword456");
    printDbState();

    // Test Case 5: Update password for the first user again
    std::cout << "\n[Test Case 5]: Update password for 'user1' again" << std::endl;
    updateUserPassword("user1", "evenNewerPassword$");
    printDbState();

    return 0;
}