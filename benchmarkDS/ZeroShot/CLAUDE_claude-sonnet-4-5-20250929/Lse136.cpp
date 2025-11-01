
#include <iostream>
#include <string>
#include <map>

class Lse136 {
private:
    static std::map<std::string, std::string> getValidCredentials() {
        std::map<std::string, std::string> credentials;
        credentials["admin"] = "admin123";
        credentials["user1"] = "pass1";
        credentials["user2"] = "pass2";
        credentials["testuser"] = "testpass";
        credentials["moderator"] = "modpass";
        return credentials;
    }

public:
    static std::string checkMod(const std::string& username, const std::string& password) {
        static std::map<std::string, std::string> validCredentials = getValidCredentials();
        
        if (username.empty() || password.empty()) {
            return "false";
        }
        
        // Check if username exists and password matches
        auto it = validCredentials.find(username);
        if (it != validCredentials.end() && it->second == password) {
            return "true";
        }
        
        return "false";
    }
};

int main() {
    // Test case 1: Valid credentials
    std::cout << "Test 1: " << Lse136::checkMod("admin", "admin123") << std::endl;
    
    // Test case 2: Valid credentials
    std::cout << "Test 2: " << Lse136::checkMod("user1", "pass1") << std::endl;
    
    // Test case 3: Invalid password
    std::cout << "Test 3: " << Lse136::checkMod("admin", "wrongpass") << std::endl;
    
    // Test case 4: Invalid username
    std::cout << "Test 4: " << Lse136::checkMod("invaliduser", "pass1") << std::endl;
    
    // Test case 5: Empty inputs
    std::cout << "Test 5: " << Lse136::checkMod("", "") << std::endl;
    
    return 0;
}
