#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

// In-memory "database". In a real application, this would be a persistent database.
static std::unordered_map<std::string, std::string> userDatabase;

/**
 * @brief Hashes a password using a simple, non-cryptographic algorithm.
 * @note In a real-world application, use a well-vetted cryptographic library
 *       like OpenSSL, libsodium, or bcrypt for password hashing. This simple
 *       hash is for demonstration purposes only.
 * @param password The plain-text password to hash.
 * @return A string representation of the hash.
 */
std::string hashPassword(const std::string& password) {
    // Using DJB2 hash algorithm for simplicity
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

/**
 * @brief Registers a new user by hashing their password and storing it.
 * @param username The username for the new account.
 * @param password The plain-text password for the new account.
 * @return true if registration is successful, false otherwise.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (userDatabase.find(username) != userDatabase.end()) {
        std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }
    std::string hashedPassword = hashPassword(password);
    userDatabase[username] = hashedPassword;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

int main() {
    std::cout << "--- C++ User Registration System ---" << std::endl;
    
    // Test Case 1: Successful registration
    registerUser("alice", "password123");
    
    // Test Case 2: Another successful registration
    registerUser("bob", "bob_secure_pass");
    
    // Test Case 3: A third successful registration
    registerUser("charlie", "qwerty");
    
    // Test Case 4: Attempt to register a duplicate username
    registerUser("alice", "another_password");
    
    // Test Case 5: A final successful registration
    registerUser("david", "d@v1d_pa$$");
    
    std::cout << "\n--- Current Database ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << ", Hashed Password: " << pair.second << std::endl;
    }

    return 0;
}