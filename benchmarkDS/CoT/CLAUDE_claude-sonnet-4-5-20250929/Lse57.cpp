
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>

class Lse57 {
private:
    struct User {
        std::string email;
        std::string passwordHash;
        std::string salt;
    };
    
    std::map<std::string, User> userDatabase;
    std::map<std::string, std::string> activeSessions;
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string combined = password + salt;
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        
        std::stringstream ss;
        for(int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
        }
        return ss.str();
    }
    
    bool isValidEmail(const std::string& email) {
        std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }
    
public:
    std::string registerUser(const std::string& email, const std::string& password) {
        if (!isValidEmail(email)) {
            return "Invalid email format";
        }
        if (password.length() < 8) {
            return "Password must be at least 8 characters";
        }
        if (userDatabase.find(email) != userDatabase.end()) {
            return "Email already exists";
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        userDatabase[email] = {email, passwordHash, salt};
        return "Registration successful";
    }
    
    std::string login(const std::string& email, const std::string& password) {
        if (userDatabase.find(email) == userDatabase.end()) {
            return "";
        }
        
        User& user = userDatabase[email];
        std::string passwordHash = hashPassword(password, user.salt);
        
        if (passwordHash == user.passwordHash) {
            std::stringstream ss;
            ss << email << std::time(nullptr);
            std::string sessionToken = hashPassword(ss.str(), generateSalt());
            activeSessions[sessionToken] = email;
            return sessionToken;
        }
        return "";
    }
    
    std::string changeEmail(const std::string& sessionToken, const std::string& oldEmail,
                           const std::string& newEmail, const std::string& confirmPassword) {
        // Verify session
        if (activeSessions.find(sessionToken) == activeSessions.end()) {
            return "User not logged in";
        }
        
        std::string loggedInEmail = activeSessions[sessionToken];
        
        // Verify old email matches logged in user
        if (loggedInEmail != oldEmail) {
            return "Old email does not match logged in user";
        }
        
        // Verify old email exists
        if (userDatabase.find(oldEmail) == userDatabase.end()) {
            return "Old email does not exist";
        }
        
        // Validate new email format
        if (!isValidEmail(newEmail)) {
            return "Invalid new email format";
        }
        
        // Check if new email already exists
        if (userDatabase.find(newEmail) != userDatabase.end()) {
            return "New email already exists";
        }
        
        // Verify confirm password
        User& user = userDatabase[oldEmail];
        std::string passwordHash = hashPassword(confirmPassword, user.salt);
        
        if (passwordHash != user.passwordHash) {
            return "Incorrect password";
        }
        
        // Change email
        user.email = newEmail;
        userDatabase[newEmail] = user;
        userDatabase.erase(oldEmail);
        activeSessions[sessionToken] = newEmail;
        
        return "Email changed successfully";
    }
};

int main() {
    Lse57 system;
    
    std::cout << "=== Test Case 1: Successful email change ===" << std::endl;
    system.registerUser("user1@example.com", "password123");
    std::string token1 = system.login("user1@example.com", "password123");
    std::cout << system.changeEmail(token1, "user1@example.com", 
                                   "newuser1@example.com", "password123") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Invalid session token ===" << std::endl;
    std::cout << system.changeEmail("invalid_token", "user2@example.com", 
                                   "newuser2@example.com", "password123") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Incorrect password ===" << std::endl;
    system.registerUser("user3@example.com", "password123");
    std::string token3 = system.login("user3@example.com", "password123");
    std::cout << system.changeEmail(token3, "user3@example.com", 
                                   "newuser3@example.com", "wrongpassword") << std::endl;
    
    std::cout << "\\n=== Test Case 4: New email already exists ===" << std::endl;
    system.registerUser("user4@example.com", "password123");
    system.registerUser("existing@example.com", "password123");
    std::string token4 = system.login("user4@example.com", "password123");
    std::cout << system.changeEmail(token4, "user4@example.com", 
                                   "existing@example.com", "password123") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Invalid new email format ===" << std::endl;
    system.registerUser("user5@example.com", "password123");
    std::string token5 = system.login("user5@example.com", "password123");
    std::cout << system.changeEmail(token5, "user5@example.com", 
                                   "invalid-email", "password123") << std::endl;
    
    return 0;
}
