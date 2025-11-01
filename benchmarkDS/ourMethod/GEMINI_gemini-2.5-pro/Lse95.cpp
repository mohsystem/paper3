#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// In a real application, this would be a database.
static std::map<std::string, std::vector<unsigned char>> userDatabase;

// Use strong, modern parameters for PBKDF2.
const int ITERATIONS = 310000;
const int KEY_LEN_BYTES = 32; // 256 bits
const int SALT_LEN_BYTES = 16; // 128 bits

/**
 * @brief Registers a new user.
 * @param username The user's name.
 * @param password The user's password.
 * @return True on success, false on failure (e.g., user exists).
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Username and password cannot be empty." << std::endl;
        return false;
    }

    if (userDatabase.count(username)) {
        std::cout << "User '" << username << "' already exists." << std::endl;
        return false;
    }

    // Generate a cryptographically secure random salt.
    std::vector<unsigned char> salt(SALT_LEN_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }
    
    // Hash the password using PBKDF2-HMAC-SHA256.
    std::vector<unsigned char> hash(KEY_LEN_BYTES);
    if (PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(),
        salt.data(), 
        salt.size(),
        ITERATIONS,
        EVP_sha256(),
        hash.size(), 
        hash.data()
    ) != 1) {
        throw std::runtime_error("Failed to hash password.");
    }
    
    // Combine salt and hash for storage.
    std::vector<unsigned char> combined = salt;
    combined.insert(combined.end(), hash.begin(), hash.end());
    
    userDatabase[username] = combined;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

/**
 * @brief Verifies a user's login credentials.
 * @param username The user's name.
 * @param password The password to verify.
 * @return True if the password is correct, false otherwise.
 */
bool loginUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cout << "Login failed: User '" << username << "' not found." << std::endl;
        return false; // User not found
    }

    const std::vector<unsigned char>& combined = it->second;
    if (combined.size() != SALT_LEN_BYTES + KEY_LEN_BYTES) {
        std::cerr << "Login failed: Invalid stored hash format for user '" << username << "'." << std::endl;
        return false;
    }

    // Extract salt and stored hash.
    std::vector<unsigned char> salt(combined.begin(), combined.begin() + SALT_LEN_BYTES);
    std::vector<unsigned char> storedHash(combined.begin() + SALT_LEN_BYTES, combined.end());

    // Hash the provided password with the retrieved salt.
    std::vector<unsigned char> calculatedHash(KEY_LEN_BYTES);
    if (PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(),
        salt.data(), 
        salt.size(),
        ITERATIONS,
        EVP_sha256(),
        calculatedHash.size(), 
        calculatedHash.data()
    ) != 1) {
        throw std::runtime_error("Failed to hash password during login.");
    }

    // Compare hashes in a constant-time manner.
    bool match = (CRYPTO_memcmp(storedHash.data(), calculatedHash.data(), KEY_LEN_BYTES) == 0);
    
    if (match) {
        std::cout << "Login successful for user '" << username << "'." << std::endl;
    } else {
        std::cout << "Login failed: Invalid password for user '" << username << "'." << std::endl;
    }

    return match;
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    try {
        // Test Case 1: Successful registration
        std::cout << "\n[Test 1] Registering a new user 'alice'." << std::endl;
        registerUser("alice", "P@ssw0rd123!");

        // Test Case 2: Attempting to register a user that already exists
        std::cout << "\n[Test 2] Attempting to re-register 'alice'." << std::endl;
        registerUser("alice", "anotherpassword");

        // Test Case 3: Successful login
        std::cout << "\n[Test 3] Alice logs in with the correct password." << std::endl;
        loginUser("alice", "P@ssw0rd123!");

        // Test Case 4: Failed login (wrong password)
        std::cout << "\n[Test 4] Alice attempts to log in with an incorrect password." << std::endl;
        loginUser("alice", "wrongpassword");

        // Test Case 5: Failed login (user not found)
        std::cout << "\n[Test 5] Attempting to log in as a non-existent user 'bob'." << std::endl;
        loginUser("bob", "anypassword");

    } catch (const std::runtime_error& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n--- Test Cases Finished ---" << std::endl;

    return 0;
}