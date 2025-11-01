#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

// User struct to hold user data
struct User {
    std::string username;
    std::string hashedPassword;
    std::string firstName;
    std::string lastName;
    std::string email;
};

// In-memory map to simulate a database
std::unordered_map<std::string, User> database;

/**
 * A simple, non-cryptographic "hashing" function for demonstration purposes.
 * In a real application, use a proper cryptographic library like OpenSSL or Crypto++.
 * This function calculates a DJB2 hash and returns it as a hex string.
 * @param password The password to hash.
 * @return The string representation of the hash.
 */
std::string hashPassword(const std::string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

/**
 * Registers a new user.
 * @param username The username.
 * @param password The plain text password.
 * @param firstName The user's first name.
 * @param lastName The user's last name.
 * @param email The user's email.
 * @return A message indicating success or failure.
 */
std::string registerUser(const std::string& username, const std::string& password, const std::string& firstName, const std::string& lastName, const std::string& email) {
    if (database.count(username)) {
        return "Username '" + username + "' already exists.";
    } else {
        std::string hashedPassword = hashPassword(password);
        User newUser = {username, hashedPassword, firstName, lastName, email};
        database[username] = newUser;
        return "Registration succeeded for username '" + username + "'.";
    }
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Successful registration
    std::string result1 = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
    std::cout << "Test Case 1: " << result1 << std::endl;

    // Test Case 2: Another successful registration
    std::string result2 = registerUser("asmith", "SecurePass!", "Alice", "Smith", "asmith@example.com");
    std::cout << "Test Case 2: " << result2 << std::endl;

    // Test Case 3: Attempt to register with an existing username
    std::string result3 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
    std::cout << "Test Case 3: " << result3 << std::endl;

    // Test Case 4: A third successful registration
    std::string result4 = registerUser("peterpan", "neverland", "Peter", "Pan", "peter.p@example.com");
    std::cout << "Test Case 4: " << result4 << std::endl;

    // Test Case 5: Another failed registration with a duplicate username
    std::string result5 = registerUser("asmith", "MyPass123", "Adam", "Smithy", "a.smith@example.com");
    std::cout << "Test Case 5: " << result5 << std::endl;

    // Optional: Print the database content to verify
    // std::cout << "\nCurrent database state:" << std::endl;
    // for (const auto& pair : database) {
    //     std::cout << pair.first << " -> Hashed Pwd: " << pair.second.hashedPassword << std::endl;
    // }

    return 0;
}