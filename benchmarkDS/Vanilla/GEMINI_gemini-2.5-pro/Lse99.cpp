#include <iostream>
#include <string>
#include <unordered_map>

// Simulating a database with an unordered_map
static std::unordered_map<std::string, std::string> database;

/**
 * A simple, non-cryptographically secure hashing function for demonstration.
 * In a real application, use a proper library like OpenSSL or Crypto++.
 * This function uses the djb2 algorithm.
 * @param password The password to hash.
 * @return A string representation of the hash.
 */
std::string hashPassword(const std::string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return std::to_string(hash);
}

/**
 * "Inserts" a new user into the simulated database.
 * @param username The username for the new user.
 * @param password The password for the new user.
 * @return true if the user was successfully registered, false otherwise.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Error: Username and password cannot be empty." << std::endl;
        return false;
    }
    if (database.count(username)) {
        std::cout << "Error: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    std::string hashedPassword = hashPassword(password);
    database[username] = hashedPassword;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

int main() {
    std::cout << "--- Running 5 User Registration Test Cases ---" << std::endl;

    // Test Case 1
    registerUser("alice", "password123");

    // Test Case 2
    registerUser("bob", "my$ecretP@ss");

    // Test Case 3
    registerUser("charlie", "Charlie'sPassword!");

    // Test Case 4
    registerUser("david", "d@v1d_R0ck$");

    // Test Case 5
    registerUser("eve", "spyware");

    std::cout << "\n--- Final Database State ---" << std::endl;
    for (const auto& pair : database) {
        std::cout << "Username: " << pair.first << ", Hashed Password: " << pair.second << std::endl;
    }

    return 0;
}