#include <iostream>
#include <string>
#include <unordered_map>

// User structure to hold user data
struct User {
    std::string name;
    std::string email;
    std::string password; // In a real app, this should be hashed
};

// Simulating a database using an unordered_map for efficient lookups
std::unordered_map<std::string, User> userDatabase;

/**
 * @brief Registers a new user in the database.
 * 
 * @param name The user's name.
 * @param email The user's email (must be unique).
 * @param password The user's password.
 * @return true if registration is successful, false if email already exists.
 */
bool registerUser(const std::string& name, const std::string& email, const std::string& password) {
    if (userDatabase.count(email) > 0) {
        std::cout << "Registration failed: Email '" << email << "' already exists." << std::endl;
        return false;
    }
    User newUser = {name, email, password};
    userDatabase[email] = newUser;
    std::cout << "Registration successful for: User{name='" << name << "', email='" << email << "'}" << std::endl;
    return true;
}

int main() {
    std::cout << "--- User Registration System ---" << std::endl;

    // Test Case 1: Successful registration
    std::cout << "\nTest Case 1:" << std::endl;
    registerUser("Alice", "alice@example.com", "password123");

    // Test Case 2: Successful registration
    std::cout << "\nTest Case 2:" << std::endl;
    registerUser("Bob", "bob@example.com", "securepass");

    // Test Case 3: Failed registration (duplicate email)
    std::cout << "\nTest Case 3:" << std::endl;
    registerUser("Alice Smith", "alice@example.com", "anotherpass");

    // Test Case 4: Successful registration
    std::cout << "\nTest Case 4:" << std::endl;
    registerUser("Charlie", "charlie@example.com", "charlie!@#");
    
    // Test Case 5: Successful registration
    std::cout << "\nTest Case 5:" << std::endl;
    registerUser("Diana", "diana@example.com", "diana_pass");

    std::cout << "\n--- Current Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Email: " << pair.first << ", User: {name='" << pair.second.name << "'}" << std::endl;
    }

    return 0;
}