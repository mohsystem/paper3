#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm> // For std::reverse

// User struct to hold user information
struct User {
    std::string username;
    std::string hashedPassword;
    std::string firstName;
    std::string lastName;
    std::string email;
};

// In-memory map to act as a database
std::unordered_map<std::string, User> database;

/**
 * !SECURITY WARNING!
 * This is a placeholder hashing function for demonstration purposes ONLY.
 * It is NOT secure. In a real-world C++ application, you MUST use a
 * well-vetted cryptographic library like OpenSSL, Crypto++, or Botan
 * to implement a strong hashing algorithm like Argon2, scrypt, or bcrypt.
 *
 * @param password The password to "hash".
 * @return A non-secure, reversed string representation of the password.
 */
std::string hashPassword(const std::string& password) {
    // This is a simple, insecure placeholder.
    std::string salt = "random_salt_"; // In a real app, this should be unique and random per user.
    std::string saltedPassword = salt + password;
    std::reverse(saltedPassword.begin(), saltedPassword.end());
    return saltedPassword;
}

/**
 * Registers a new user.
 *
 * @param username The username.
 * @param password The user's password.
 * @param firstName The user's first name.
 * @param lastName The user's last name.
 * @param email The user's email address.
 * @return A status message indicating success or failure.
 */
std::string registerUser(const std::string& username, const std::string& password, const std::string& firstName, const std::string& lastName, const std::string& email) {
    // Input validation
    if (username.empty() || password.empty()) {
        return "Registration failed: Username and password cannot be empty.";
    }

    // Check if username already exists
    if (database.find(username) != database.end()) {
        std::ostringstream oss;
        oss << "Registration failed: Username '" << username << "' already exists.";
        return oss.str();
    }

    // Hash the password
    std::string hashedPassword = hashPassword(password);

    // Create a new user and add to the "database"
    User newUser = {username, hashedPassword, firstName, lastName, email};
    database[username] = newUser;

    std::ostringstream oss;
    oss << "Registration for '" << username << "' succeeded.";
    return oss.str();
}

class Lse97 {
public:
    static void runTests() {
        std::cout << "--- User Registration System ---" << std::endl;

        // Test Case 1: Successful registration
        std::string result1 = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
        std::cout << "Test Case 1: " << result1 << std::endl;

        // Test Case 2: Attempt to register with an existing username
        std::string result2 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
        std::cout << "Test Case 2: " << result2 << std::endl;

        // Test Case 3: Successful registration for another user
        std::string result3 = registerUser("asmith", "SecureP@ss!", "Alice", "Smith", "asmith@example.com");
        std::cout << "Test Case 3: " << result3 << std::endl;

        // Test Case 4: Registration with an empty password
        std::string result4 = registerUser("bwhite", "", "Bob", "White", "bwhite@example.com");
        std::cout << "Test Case 4: " << result4 << std::endl;

        // Test Case 5: Another successful registration
        std::string result5 = registerUser("cgreen", "MyP@ssw0rd", "Charlie", "Green", "cgreen@example.com");
        std::cout << "Test Case 5: " << result5 << std::endl;
        
        // Print database state for verification
        std::cout << "\nCurrent database state:" << std::endl;
        for (const auto& pair : database) {
            std::cout << "Username: " << pair.second.username 
                      << ", Hashed Password: " << pair.second.hashedPassword << std::endl;
        }
    }
};

int main() {
    Lse97::runTests();
    return 0;
}