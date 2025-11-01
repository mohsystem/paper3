
#include <iostream>
#include <string>
#include <map>

class Lse135 {
private:
    static std::map<std::string, std::string> userDatabase;
    
public:
    /**
     * Checks if the provided username and password are correct
     * @param username The username from the form
     * @param password The password from the form
     * @return true if credentials are correct, false otherwise
     */
    static bool checkMod(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        
        return it->second == password;
    }
    
    static void initializeDatabase() {
        userDatabase["admin"] = "admin123";
        userDatabase["user1"] = "password1";
        userDatabase["moderator"] = "mod123";
        userDatabase["john"] = "doe456";
        userDatabase["alice"] = "wonderland";
    }
};

// Initialize static member
std::map<std::string, std::string> Lse135::userDatabase;

int main() {
    Lse135::initializeDatabase();
    
    // Test case 1: Valid credentials
    std::cout << "Test 1 - Valid user (admin/admin123): " 
              << (Lse135::checkMod("admin", "admin123") ? "true" : "false") << std::endl;
    
    // Test case 2: Valid credentials
    std::cout << "Test 2 - Valid user (user1/password1): " 
              << (Lse135::checkMod("user1", "password1") ? "true" : "false") << std::endl;
    
    // Test case 3: Invalid password
    std::cout << "Test 3 - Invalid password (admin/wrong): " 
              << (Lse135::checkMod("admin", "wrong") ? "true" : "false") << std::endl;
    
    // Test case 4: Non-existent user
    std::cout << "Test 4 - Non-existent user (hacker/hack): " 
              << (Lse135::checkMod("hacker", "hack") ? "true" : "false") << std::endl;
    
    // Test case 5: Empty values
    std::cout << "Test 5 - Empty values: " 
              << (Lse135::checkMod("", "") ? "true" : "false") << std::endl;
    
    return 0;
}
