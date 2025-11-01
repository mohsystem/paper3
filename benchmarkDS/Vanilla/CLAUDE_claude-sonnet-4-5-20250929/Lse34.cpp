
#include <iostream>
#include <string>
#include <map>

class Lse34 {
private:
    std::map<std::string, std::string> database;
    
public:
    Lse34() {
        // Initialize mock database
        database["admin"] = "admin123";
        database["user1"] = "password1";
        database["user2"] = "password2";
        database["testuser"] = "testpass";
        database["john"] = "doe123";
    }
    
    std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "login";
        }
        
        auto it = database.find(username);
        if (it != database.end() && it->second == password) {
            return "dashboard";
        } else {
            return "login";
        }
    }
};

int main() {
    Lse34 lse;
    
    // Test case 1: Valid credentials
    std::cout << "Test 1 - Valid user (admin/admin123): " << lse.login("admin", "admin123") << std::endl;
    
    // Test case 2: Invalid password
    std::cout << "Test 2 - Invalid password (admin/wrongpass): " << lse.login("admin", "wrongpass") << std::endl;
    
    // Test case 3: Non-existent user
    std::cout << "Test 3 - Non-existent user (unknown/pass): " << lse.login("unknown", "pass") << std::endl;
    
    // Test case 4: Valid credentials (user1)
    std::cout << "Test 4 - Valid user (user1/password1): " << lse.login("user1", "password1") << std::endl;
    
    // Test case 5: Empty credentials
    std::cout << "Test 5 - Empty credentials: " << lse.login("", "") << std::endl;
    
    return 0;
}
