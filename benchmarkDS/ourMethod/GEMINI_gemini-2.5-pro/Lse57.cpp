#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <regex>

// NOTE: This is a simplified User struct. In a real application, memory for
// passwordHash and salt should be securely managed and cleared.
struct User {
    std::string username;
    std::string email;
    std::string passwordHash; // In a real application, this would be a byte array.
    std::string salt;         // In a real application, this would be a byte array.
};

class UserService {
private:
    std::unordered_map<std::string, User> users;
    User* loggedInUser = nullptr;
    // Simple regex for email validation
    const std::regex email_pattern{"^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$"};

    // WARNING: This is a placeholder for a real cryptographic salt generator.
    // In a production environment, use a cryptographically secure pseudo-random
    // number generator (CSPRNG) from a library like OpenSSL or the OS's specific API.
    std::string generateSalt(size_t length) {
        const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device random_device;
        std::mt19937 generator(random_device());
        std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
        
        std::string random_string;
        for (size_t i = 0; i < length; ++i) {
            random_string += CHARACTERS[distribution(generator)];
        }
        return random_string;
    }
    
    // WARNING: This is NOT a secure password hash. It's a simple placeholder.
    // In a production system, you MUST use a standard, strong key derivation function
    // like Argon2, scrypt, or PBKDF2 from a vetted cryptography library (e.g., OpenSSL).
    std::string hashPassword(const std::string& password, const std::string& salt) {
        // This simple hash just concatenates and uses std::hash for demonstration.
        // DO NOT USE THIS IN PRODUCTION.
        std::hash<std::string> hasher;
        return std::to_string(hasher(password + salt));
    }

    bool verifyPassword(const std::string& password, const std::string& originalHash, const std::string& salt) {
        return originalHash == hashPassword(password, salt);
    }


public:
    ~UserService() {
        // In a real application with dynamic allocation for users, clear memory here.
    }

    bool registerUser(const std::string& username, const std::string& password, const std::string& email) {
        // Rule #1: Input Validation
        if (username.empty() || password.empty() || email.empty() || !std::regex_match(email, email_pattern)) {
            std::cout << "Registration failed: Invalid input." << std::endl;
            return false;
        }
        if (users.count(username)) {
            std::cout << "Registration failed: Username already exists." << std::endl;
            return false;
        }

        std::string salt = generateSalt(16);
        std::string hash = hashPassword(password, salt);

        User newUser{username, email, hash, salt};
        users[username] = newUser;

        std::cout << "User " << username << " registered successfully." << std::endl;
        return true;
    }

    bool login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        auto it = users.find(username);
        if (it != users.end() && verifyPassword(password, it->second.passwordHash, it->second.salt)) {
            loggedInUser = &it->second;
            std::cout << username << " logged in successfully." << std::endl;
            return true;
        }
        std::cout << "Login failed for " << username << "." << std::endl;
        return false;
    }
    
    void logout() {
        if (loggedInUser) {
            std::cout << loggedInUser->username << " logged out." << std::endl;
            loggedInUser = nullptr;
        }
    }

    bool changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
        // Rule #1: Input Validation
        if (oldEmail.empty() || newEmail.empty() || confirmPassword.empty() || !std::regex_match(newEmail, email_pattern)) {
            std::cout << "Email change failed: Invalid input provided." << std::endl;
            return false;
        }

        if (!loggedInUser) {
            std::cout << "Email change failed: User must be logged in." << std::endl;
            return false;
        }
        
        if (loggedInUser->email != oldEmail) {
            std::cout << "Email change failed: Old email does not match." << std::endl;
            return false;
        }

        if (!verifyPassword(confirmPassword, loggedInUser->passwordHash, loggedInUser->salt)) {
            std::cout << "Email change failed: Incorrect password." << std::endl;
            return false;
        }

        loggedInUser->email = newEmail;
        std::cout << "Email for user " << loggedInUser->username << " changed successfully to " << newEmail << std::endl;
        return true;
    }
    
    void printUserDetails(const std::string& username) {
        auto it = users.find(username);
        if (it != users.end()) {
            std::cout << "Details for " << username << ": Email = " << it->second.email << std::endl;
        } else {
            std::cout << "User " << username << " not found." << std::endl;
        }
    }
};

int main() {
    UserService service;
    
    // Setup: Register a user
    service.registerUser("cpp_user", "CppPass123!", "initial_cpp@example.com");
    std::cout << "----------------------------------------" << std::endl;
    
    // Test Case 1: Successful email change
    std::cout << "Test Case 1: Successful email change" << std::endl;
    service.login("cpp_user", "CppPass123!");
    service.printUserDetails("cpp_user");
    service.changeEmail("initial_cpp@example.com", "updated_cpp@example.com", "CppPass123!");
    service.printUserDetails("cpp_user");
    service.logout();
    std::cout << "----------------------------------------" << std::endl;

    // Test Case 2: Attempt to change email when not logged in
    std::cout << "Test Case 2: Attempt change when not logged in" << std::endl;
    service.changeEmail("updated_cpp@example.com", "another_cpp@example.com", "CppPass123!");
    service.printUserDetails("cpp_user");
    std::cout << "----------------------------------------" << std::endl;

    // Test Case 3: Attempt to change email with wrong password
    std::cout << "Test Case 3: Attempt change with wrong password" << std::endl;
    service.login("cpp_user", "CppPass123!");
    service.changeEmail("updated_cpp@example.com", "another_cpp@example.com", "WrongPassword!");
    service.printUserDetails("cpp_user");
    service.logout();
    std::cout << "----------------------------------------" << std::endl;

    // Test Case 4: Attempt to change email with wrong old email
    std::cout << "Test Case 4: Attempt change with wrong old email" << std::endl;
    service.login("cpp_user", "CppPass123!");
    service.changeEmail("wrong.old_cpp@example.com", "another_cpp@example.com", "CppPass123!");
    service.printUserDetails("cpp_user");
    service.logout();
    std::cout << "----------------------------------------" << std::endl;
    
    // Test Case 5: Attempt to change to an invalid new email
    std::cout << "Test Case 5: Attempt change to an invalid new email" << std::endl;
    service.login("cpp_user", "CppPass123!");
    service.changeEmail("updated_cpp@example.com", "invalid-email", "CppPass123!");
    service.printUserDetails("cpp_user");
    service.logout();
    std::cout << "----------------------------------------" << std::endl;
    
    return 0;
}