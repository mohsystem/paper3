#include <iostream>
#include <string>
#include <map>
#include <vector>

// In-memory map to simulate a user database.
// In a real application, use a persistent database (e.g., PostgreSQL, MySQL).
static std::map<std::string, std::string> userDatabase;

/**
 * SECURE HASHING PLACEHOLDER
 *
 * WARNING: THIS IS NOT A SECURE HASHING FUNCTION.
 * C++ does not have a standard library for cryptography. For a real-world,
 * secure application, you MUST use a well-vetted, external library like
 * OpenSSL, Crypto++, or libsodium.
 *
 * This function is a placeholder to demonstrate the application's logic flow.
 * It does not provide any security.
 */
std::string insecure_placeholder_hash(const std::string& password) {
    // This is a dummy salt. A real implementation would generate a unique,
    // cryptographically random salt for each user.
    std::string salt = "dummy_salt_for_";
    return salt + password; // Insecure: simply concatenates salt and password
}

/**
 * Registers a new user by "hashing" their password and storing it.
 * @param username The username for the new account.
 * @param password The plain-text password for the new account.
 * @return true if registration is successful, false otherwise.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
        return false;
    }

    // Check if the user already exists
    if (userDatabase.find(username) != userDatabase.end()) {
        std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    // "Hash" the password using the insecure placeholder function
    std::string hashedPassword = insecure_placeholder_hash(password);

    // Insert the new user into our simulated database
    userDatabase[username] = hashedPassword;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

int main() {
    std::cout << "--- Running 5 Test Cases for User Registration ---" << std::endl;
    std::cout << "WARNING: Using an insecure placeholder for password hashing." << std::endl;

    // Test Case 1: Successful registration
    registerUser("alice", "Password123!");

    // Test Case 2: Successful registration with a different user
    registerUser("bob", "SecurePass@2023");

    // Test Case 3: Attempt to register a duplicate username
    registerUser("alice", "AnotherPassword");

    // Test Case 4: Registration with an empty password
    registerUser("charlie", "");

    // Test Case 5: Successful registration
    registerUser("david", "MyP@ssw0rd");

    std::cout << "\n--- Final Simulated Database State ---" << std::endl;
    // Note: In a real application, NEVER print sensitive data like this.
    // This is for demonstration purposes only.
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << ", Stored Value: " << pair.second << std::endl;
    }

    return 0;
}