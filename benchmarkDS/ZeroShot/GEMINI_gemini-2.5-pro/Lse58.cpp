#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>

// For demonstration purposes, we'll simulate hashing.
// For a real application, use a proper cryptographic library like OpenSSL or Crypto++.
#include <algorithm> // For std::reverse
#include <functional> // For std::hash

class Lse58 {
private:
    struct User {
        std::string username;
        // In a real application, never store the password directly.
        // Store a strong, salted hash (e.g., using Argon2, scrypt, bcrypt).
        size_t hashedPassword;
        std::string salt;
        std::string email;
    };

    std::map<std::string, User> users;
    std::string currentUsername;

    /**
     * Creates a simple "salt" from the username.
     * WARNING: In a real application, use a cryptographically secure random generator
     * to create a unique salt for each user.
     */
    std::string createSalt(const std::string& username) {
        std::string salt = username;
        std::reverse(salt.begin(), salt.end());
        return salt + "_s@lt";
    }

    /**
     * Hashes a password with a given salt.
     * WARNING: This is NOT a secure hashing function. It is for demonstration only.
     * For a real-world application, use a stronger, adaptive hashing function
     * like Argon2, scrypt, or bcrypt.
     */
    size_t hashPassword(const std::string& password, const std::string& salt) {
        std::hash<std::string> hasher;
        return hasher(password + salt);
    }

public:
    Lse58() : currentUsername("") {}

    bool registerUser(const std::string& username, const std::string& password, const std::string& email) {
        if (username.empty() || password.empty() || users.count(username)) {
            return false;
        }

        std::string salt = createSalt(username);
        size_t hashedPassword = hashPassword(password, salt);
        
        User newUser = {username, hashedPassword, salt, email};
        users[username] = newUser;
        return true;
    }

    bool login(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it != users.end()) {
            User& user = it->second;
            size_t providedPasswordHash = hashPassword(password, user.salt);
            
            // In a real application, use a constant-time comparison function
            // to prevent timing attacks.
            if (providedPasswordHash == user.hashedPassword) {
                currentUsername = username;
                return true;
            }
        }
        return false;
    }

    void logout() {
        currentUsername = "";
    }

    bool changeEmail(const std::string& oldEmail, const std::string& password, const std::string& newEmail) {
        if (currentUsername.empty()) {
            return false; // Not logged in
        }

        auto it = users.find(currentUsername);
        if (it == users.end()) {
            // Should not happen if currentUsername is valid
            return false;
        }
        
        User& user = it->second;

        if (user.email != oldEmail) {
            return false;
        }

        size_t providedPasswordHash = hashPassword(password, user.salt);
        if (providedPasswordHash != user.hashedPassword) {
            return false;
        }

        // All checks passed
        user.email = newEmail;
        return true;
    }
    
    std::string getCurrentUserEmail() {
        if (!currentUsername.empty() && users.count(currentUsername)) {
            return users[currentUsername].email;
        }
        return "N/A";
    }
};

int main() {
    Lse58 system;
    std::string testUser = "testuser";
    std::string testPass = "Password123!";
    std::string testEmail = "test@example.com";

    std::cout << "Registering user: " << testUser << std::endl;
    system.registerUser(testUser, testPass, testEmail);

    // Test Case 1: Successful login and email change.
    std::cout << "\n--- Test Case 1: Successful Change ---" << std::endl;
    bool loggedIn = system.login(testUser, testPass);
    std::cout << "Login successful: " << (loggedIn ? "true" : "false") << std::endl;
    if (loggedIn) {
        bool emailChanged = system.changeEmail(testEmail, testPass, "new@example.com");
        std::cout << "Email change successful: " << (emailChanged ? "true" : "false") << std::endl;
        std::cout << "User's new email: " << system.getCurrentUserEmail() << std::endl;
    }
    system.logout();

    // Test Case 2: Attempt to change email with the wrong password.
    std::cout << "\n--- Test Case 2: Wrong Password ---" << std::endl;
    loggedIn = system.login(testUser, testPass);
    std::cout << "Login successful: " << (loggedIn ? "true" : "false") << std::endl;
    if (loggedIn) {
        bool emailChanged = system.changeEmail("new@example.com", "WrongPassword!", "another@example.com");
        std::cout << "Email change successful: " << (emailChanged ? "true" : "false") << std::endl;
        std::cout << "User's email remains: " << system.getCurrentUserEmail() << std::endl;
    }
    system.logout();

    // Test Case 3: Attempt to change email with the wrong old email.
    std::cout << "\n--- Test Case 3: Wrong Old Email ---" << std::endl;
    loggedIn = system.login(testUser, testPass);
    std::cout << "Login successful: " << (loggedIn ? "true" : "false") << std::endl;
    if (loggedIn) {
        bool emailChanged = system.changeEmail("wrong@example.com", testPass, "another@example.com");
        std::cout << "Email change successful: " << (emailChanged ? "true" : "false") << std::endl;
        std::cout << "User's email remains: " << system.getCurrentUserEmail() << std::endl;
    }
    system.logout();

    // Test Case 4: Attempt to change email when not logged in.
    std::cout << "\n--- Test Case 4: Not Logged In ---" << std::endl;
    bool emailChanged = system.changeEmail("new@example.com", testPass, "another@example.com");
    std::cout << "Email change successful: " << (emailChanged ? "true" : "false") << std::endl;

    // Test Case 5: Successful change after a failed attempt.
    std::cout << "\n--- Test Case 5: Successful Change After Failed Attempt ---" << std::endl;
    loggedIn = system.login(testUser, testPass);
    std::cout << "Login successful: " << (loggedIn ? "true" : "false") << std::endl;
    if (loggedIn) {
        bool failedChange = system.changeEmail("new@example.com", "badpass", "final@example.com");
        std::cout << "First change attempt (should fail): " << (failedChange ? "true" : "false") << std::endl;
        bool successChange = system.changeEmail("new@example.com", testPass, "final@example.com");
        std::cout << "Second change attempt (should succeed): " << (successChange ? "true" : "false") << std::endl;
        std::cout << "User's final email: " << system.getCurrentUserEmail() << std::endl;
    }
    system.logout();

    return 0;
}