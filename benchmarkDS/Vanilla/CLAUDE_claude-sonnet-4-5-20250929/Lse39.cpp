
#include <iostream>
#include <map>
#include <string>

class Lse39 {
private:
    static std::map<std::string, std::string> userDatabase;
    
public:
    static std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "Username or password is incorrect";
        }
        
        auto it = userDatabase.find(username);
        if (it != userDatabase.end() && it->second == password) {
            return "Login successful! Redirecting to session page...";
        } else {
            return "Username or password is incorrect";
        }
    }
    
    static void initializeDatabase() {
        userDatabase["admin"] = "admin123";
        userDatabase["user1"] = "password1";
        userDatabase["user2"] = "password2";
        userDatabase["testuser"] = "test123";
        userDatabase["john"] = "doe456";
    }
};

std::map<std::string, std::string> Lse39::userDatabase;

int main() {
    Lse39::initializeDatabase();
    
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::cout << Lse39::login("admin", "admin123") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    std::cout << Lse39::login("user1", "wrongpassword") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    std::cout << Lse39::login("nonexistent", "password") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    std::cout << Lse39::login("testuser", "test123") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    std::cout << Lse39::login("john", "doe456") << std::endl;
    
    return 0;
}
