#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

// Simulating a database table with an unordered_map
static std::unordered_map<std::string, std::string> userDatabase;

/**
 * Hashes a password using a simple, NON-CRYPTOGRAPHICALLY-SECURE algorithm (djb2).
 * NOTE: For a real-world application, use a proper cryptographic library like OpenSSL or Crypto++.
 * @param password The plain text password.
 * @return A hex string representation of the hashed password.
 */
std::string hashPassword(const std::string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

/**
 * Registers a new user if the username doesn't already exist.
 * @param username The username for the new account.
 * @param password The password for the new account.
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
    std::cout << "--- C++ Registration Test ---" << std::endl;

    // Test Case 1: Successful registration
    registerUser("alice", "password123");

    // Test Case 2: Successful registration for another user
    registerUser("bob", "securePass!@#");

    // Test Case 3: Attempting to register with an existing username
    registerUser("alice", "anotherPassword");

    // Test Case 4: Successful registration with a complex username
    registerUser("charlie_1990", "my-secret-pw");
    
    // Test Case 5: Successful registration
    registerUser("david", "d@v1dP@$$");
    
    std::cout << "\n--- Current Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << ", Hashed Password: " << pair.second << std::endl;
    }

    return 0;
}