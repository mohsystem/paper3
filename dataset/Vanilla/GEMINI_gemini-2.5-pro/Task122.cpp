#include <iostream>
#include <string>
#include <unordered_map>

// A simple in-memory map to store user data.
// In a real application, this would be a database.
static std::unordered_map<std::string, std::string> userDatabase;

/**
 * @brief Signs up a new user by storing their username and password.
 * NOTE: In a real-world application, passwords should NEVER be stored in plaintext.
 * They should be securely hashed.
 *
 * @param username The username to register.
 * @param password The password for the new user.
 * @return true if registration is successful, false if the username already exists.
 */
bool signUp(const std::string& username, const std::string& password) {
    if (username.empty()) {
        std::cout << "Signup failed: Username cannot be empty." << std::endl;
        return false;
    }
    // Check if username already exists
    if (userDatabase.find(username) != userDatabase.end()) {
        std::cout << "Signup failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    // Store the new user
    userDatabase[username] = password;
    std::cout << "Signup successful: User '" << username << "' created." << std::endl;
    return true;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful signup for a new user
    signUp("alice", "password123");

    // Test Case 2: Successful signup for another new user
    signUp("bob", "securePass_456");

    // Test Case 3: Failed signup with an existing username
    signUp("alice", "anotherPassword");

    // Test Case 4: Successful signup with a user whose name is a number
    signUp("user789", "mySecret");

    // Test Case 5: Successful signup with an empty password (for demonstration)
    signUp("charlie", "");

    // Print final state of the database
    std::cout << "\nCurrent user database:" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "  - Username: " << pair.first << ", Password: " << pair.second << std::endl;
    }

    return 0;
}