
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task136 {
private:
    std::unordered_map<std::string, std::string> userDatabase;
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    Task136() {}
    
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        if (userDatabase.find(username) != userDatabase.end()) {
            return false;
        }
        std::string hashedPassword = hashPassword(password);
        userDatabase[username] = hashedPassword;
        return true;
    }
    
    bool authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty() || userDatabase.find(username) == userDatabase.end()) {
            return false;
        }
        std::string hashedPassword = hashPassword(password);
        return userDatabase[username] == hashedPassword;
    }
    
    bool deleteUser(const std::string& username) {
        if (username.empty() || userDatabase.find(username) == userDatabase.end()) {
            return false;
        }
        userDatabase.erase(username);
        return true;
    }
    
    bool changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        std::string hashedPassword = hashPassword(newPassword);
        userDatabase[username] = hashedPassword;
        return true;
    }
};

int main() {
    Task136 auth;
    
    // Test Case 1: Register a new user
    std::cout << "Test 1 - Register user 'john': " << (auth.registerUser("john", "password123") ? "true" : "false") << std::endl;
    
    // Test Case 2: Authenticate with correct credentials
    std::cout << "Test 2 - Authenticate 'john' with correct password: " << (auth.authenticateUser("john", "password123") ? "true" : "false") << std::endl;
    
    // Test Case 3: Authenticate with incorrect credentials
    std::cout << "Test 3 - Authenticate 'john' with wrong password: " << (auth.authenticateUser("john", "wrongpassword") ? "true" : "false") << std::endl;
    
    // Test Case 4: Register duplicate user
    std::cout << "Test 4 - Register duplicate user 'john': " << (auth.registerUser("john", "newpassword") ? "true" : "false") << std::endl;
    
    // Test Case 5: Change password
    std::cout << "Test 5 - Change password for 'john': " << (auth.changePassword("john", "password123", "newpassword123") ? "true" : "false") << std::endl;
    std::cout << "Test 5 - Authenticate with new password: " << (auth.authenticateUser("john", "newpassword123") ? "true" : "false") << std::endl;
    
    return 0;
}
