
#include <iostream>
#include <string>
#include <map>

class Lse136 {
private:
    static std::map<std::string, std::string> validCredentials;
    
public:
    static std::string checkMod(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "false";
        }
        
        auto it = validCredentials.find(username);
        if (it != validCredentials.end() && it->second == password) {
            return "true";
        }
        
        return "false";
    }
};

std::map<std::string, std::string> Lse136::validCredentials = {
    {"admin", "admin123"},
    {"user1", "password1"},
    {"moderator", "mod123"},
    {"testuser", "test456"},
    {"john", "john789"}
};

int main() {
    // Test case 1: Valid credentials - admin
    std::cout << "Test 1: " << Lse136::checkMod("admin", "admin123") << std::endl;
    
    // Test case 2: Valid credentials - user1
    std::cout << "Test 2: " << Lse136::checkMod("user1", "password1") << std::endl;
    
    // Test case 3: Invalid password
    std::cout << "Test 3: " << Lse136::checkMod("admin", "wrongpassword") << std::endl;
    
    // Test case 4: Invalid username
    std::cout << "Test 4: " << Lse136::checkMod("nonexistent", "password") << std::endl;
    
    // Test case 5: Empty strings
    std::cout << "Test 5: " << Lse136::checkMod("", "") << std::endl;
    
    return 0;
}
