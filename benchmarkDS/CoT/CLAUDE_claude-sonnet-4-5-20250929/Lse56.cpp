
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <regex>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <cstring>

class Lse56 {
private:
    struct User {
        std::string email;
        std::string passwordHash;
        std::string salt;
    };
    
    std::map<std::string, User> users;
    User* currentLoggedInUser;
    std::regex emailPattern;
    
    std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, sizeof(salt));
        
        std::stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
        }
        return ss.str();
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
        std::string newHash = hashPassword(password, salt);
        return CRYPTO_memcmp(newHash.c_str(), hash.c_str(), newHash.length()) == 0;
    }
    
    bool isValidEmail(const std::string& email) {
        return std::regex_match(email, emailPattern);
    }
    
public:
    Lse56() : currentLoggedInUser(nullptr), 
              emailPattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$") {}
    
    bool registerUser(const std::string& email, const std::string& password) {
        if (!isValidEmail(email) || password.empty()) {
            return false;
        }
        
        if (users.find(email) != users.end()) {
            return false;
        }
        
        User newUser;
        newUser.email = email;
        newUser.salt = generateSalt();
        newUser.passwordHash = hashPassword(password, newUser.salt);
        
        users[email] = newUser;
        return true;
    }
    
    bool login(const std::string& email, const std::string& password) {
        if (!isValidEmail(email) || password.empty()) {
            return false;
        }
        
        auto it = users.find(email);
        if (it != users.end() && verifyPassword(password, it->second.passwordHash, it->second.salt)) {
            currentLoggedInUser = &(it->second);
            return true;
        }
        return false;
    }
    
    void logout() {
        currentLoggedInUser = nullptr;
    }
    
    bool isLoggedIn() {
        return currentLoggedInUser != nullptr;
    }
    
    std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
        if (!isLoggedIn()) {
            return "Error: User must be logged in to change email";
        }
        
        if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
            return "Error: Invalid email format";
        }
        
        if (confirmPassword.empty()) {
            return "Error: Password is required";
        }
        
        if (currentLoggedInUser->email != oldEmail) {
            return "Error: Old email does not match current user email";
        }
        
        if (oldEmail == newEmail) {
            return "Error: New email must be different from old email";
        }
        
        if (!verifyPassword(confirmPassword, currentLoggedInUser->passwordHash, currentLoggedInUser->salt)) {
            return "Error: Incorrect password";
        }
        
        if (users.find(newEmail) != users.end()) {
            return "Error: New email already exists";
        }
        
        User tempUser = *currentLoggedInUser;
        users.erase(oldEmail);
        tempUser.email = newEmail;
        users[newEmail] = tempUser;
        currentLoggedInUser = &users[newEmail];
        
        return "Success: Email changed successfully";
    }
};

int main() {
    Lse56 system;
    
    // Test Case 1: Register and login successfully
    system.registerUser("user@example.com", "SecurePass123");
    std::cout << "Test 1 - Login: " << (system.login("user@example.com", "SecurePass123") ? "true" : "false") << std::endl;
    
    // Test Case 2: Change email successfully
    std::cout << "Test 2 - Change Email: " << system.changeEmail("user@example.com", "newemail@example.com", "SecurePass123") << std::endl;
    
    // Test Case 3: Try to change email with wrong password
    system.logout();
    system.login("newemail@example.com", "SecurePass123");
    std::cout << "Test 3 - Wrong Password: " << system.changeEmail("newemail@example.com", "another@example.com", "WrongPass") << std::endl;
    
    // Test Case 4: Try to change email to same email
    std::cout << "Test 4 - Same Email: " << system.changeEmail("newemail@example.com", "newemail@example.com", "SecurePass123") << std::endl;
    
    // Test Case 5: Try to change email without being logged in
    system.logout();
    std::cout << "Test 5 - Not Logged In: " << system.changeEmail("newemail@example.com", "test@example.com", "SecurePass123") << std::endl;
    
    return 0;
}
