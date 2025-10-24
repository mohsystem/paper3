
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task133 {
private:
    struct User {
        std::string username;
        std::string email;
        std::string passwordHash;
        
        User() {}
        User(const std::string& u, const std::string& e, const std::string& p)
            : username(u), email(e), passwordHash(p) {}
    };
    
    struct ResetToken {
        std::string token;
        std::string username;
        long long expiryTime;
        
        ResetToken() : expiryTime(0) {}
        ResetToken(const std::string& t, const std::string& u, long long e)
            : token(t), username(u), expiryTime(e) {}
        
        bool isValid() const {
            return static_cast<long long>(time(nullptr) * 1000) < expiryTime;
        }
    };
    
    std::map<std::string, User> users;
    std::map<std::string, ResetToken> resetTokens;
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    std::string generateToken(const std::string& username) {
        std::stringstream ss;
        ss << username << time(nullptr) << rand();
        return hashPassword(ss.str()).substr(0, 32);
    }
    
public:
    Task133() {
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    
    bool registerUser(const std::string& username, const std::string& email, 
                     const std::string& password) {
        if (users.find(username) != users.end()) {
            return false;
        }
        std::string passwordHash = hashPassword(password);
        users[username] = User(username, email, passwordHash);
        return true;
    }
    
    std::string requestPasswordReset(const std::string& username) {
        if (users.find(username) == users.end()) {
            return "";
        }
        
        std::string token = generateToken(username);
        long long expiryTime = time(nullptr) * 1000 + 3600000; // 1 hour
        resetTokens[token] = ResetToken(token, username, expiryTime);
        return token;
    }
    
    bool resetPassword(const std::string& token, const std::string& newPassword) {
        if (resetTokens.find(token) == resetTokens.end()) {
            return false;
        }
        
        ResetToken resetToken = resetTokens[token];
        if (!resetToken.isValid()) {
            resetTokens.erase(token);
            return false;
        }
        
        if (users.find(resetToken.username) == users.end()) {
            return false;
        }
        
        users[resetToken.username].passwordHash = hashPassword(newPassword);
        resetTokens.erase(token);
        return true;
    }
    
    bool verifyPassword(const std::string& username, const std::string& password) {
        if (users.find(username) == users.end()) {
            return false;
        }
        return users[username].passwordHash == hashPassword(password);
    }
};

int main() {
    Task133 passwordReset;
    
    // Test Case 1: Register a user and verify password
    std::cout << "Test Case 1: Register user" << std::endl;
    bool registered = passwordReset.registerUser("john_doe", "john@example.com", "password123");
    std::cout << "User registered: " << (registered ? "true" : "false") << std::endl;
    std::cout << "Password verified: " << (passwordReset.verifyPassword("john_doe", "password123") ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Request password reset
    std::cout << "Test Case 2: Request password reset" << std::endl;
    std::string token = passwordReset.requestPasswordReset("john_doe");
    std::cout << "Reset token generated: " << (!token.empty() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Reset password with valid token
    std::cout << "Test Case 3: Reset password with valid token" << std::endl;
    bool resetSuccess = passwordReset.resetPassword(token, "newPassword456");
    std::cout << "Password reset successful: " << (resetSuccess ? "true" : "false") << std::endl;
    std::cout << "Old password verified: " << (passwordReset.verifyPassword("john_doe", "password123") ? "true" : "false") << std::endl;
    std::cout << "New password verified: " << (passwordReset.verifyPassword("john_doe", "newPassword456") ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Try to reset with invalid token
    std::cout << "Test Case 4: Reset with invalid token" << std::endl;
    bool invalidReset = passwordReset.resetPassword("invalidtoken123", "anotherPassword");
    std::cout << "Reset with invalid token: " << (invalidReset ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Request reset for non-existent user
    std::cout << "Test Case 5: Request reset for non-existent user" << std::endl;
    std::string invalidToken = passwordReset.requestPasswordReset("non_existent_user");
    std::cout << "Token for non-existent user: " << (!invalidToken.empty() ? "true" : "false") << std::endl;
    
    return 0;
}
