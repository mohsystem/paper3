
#include <iostream>
#include <string>
#include <map>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <cstring>

class Lse37 {
private:
    static std::map<std::string, std::string> userDatabase;
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    static void initializeDatabase() {
        if (userDatabase.empty()) {
            userDatabase["admin"] = hashPassword("admin123");
            userDatabase["user1"] = hashPassword("password1");
            userDatabase["john"] = hashPassword("securePass");
            userDatabase["alice"] = hashPassword("alice2023");
            userDatabase["bob"] = hashPassword("bobSecret");
        }
    }
    
public:
    static std::string login(const std::string& username, const std::string& password) {
        try {
            initializeDatabase();
            
            // Input validation
            if (username.empty() || password.empty()) {
                return "cant_login";
            }
            
            // Trim username
            std::string trimmedUsername = username;
            trimmedUsername.erase(0, trimmedUsername.find_first_not_of(" \\t\\n\\r"));
            trimmedUsername.erase(trimmedUsername.find_last_not_of(" \\t\\n\\r") + 1);
            
            if (trimmedUsername.empty()) {
                return "cant_login";
            }
            
            // Hash the provided password
            std::string hashedPassword = hashPassword(password);
            
            // Check credentials
            auto it = userDatabase.find(trimmedUsername);
            
            if (it != userDatabase.end() && it->second == hashedPassword) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Login error: " << e.what() << std::endl;
            return "internal_server_error";
        } catch (...) {
            std::cerr << "Unknown login error" << std::endl;
            return "internal_server_error";
        }
    }
};

std::map<std::string, std::string> Lse37::userDatabase;

int main() {
    std::cout << "=== Secure Login System Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid credentials
    std::string result1 = Lse37::login("admin", "admin123");
    std::cout << "Test 1 - Valid login (admin): " << result1 << std::endl;
    
    // Test Case 2: Invalid password
    std::string result2 = Lse37::login("admin", "wrongpass");
    std::cout << "Test 2 - Invalid password: " << result2 << std::endl;
    
    // Test Case 3: Non-existent user
    std::string result3 = Lse37::login("hacker", "password");
    std::cout << "Test 3 - Non-existent user: " << result3 << std::endl;
    
    // Test Case 4: Empty credentials
    std::string result4 = Lse37::login("", "");
    std::cout << "Test 4 - Empty credentials: " << result4 << std::endl;
    
    // Test Case 5: Valid user different password
    std::string result5 = Lse37::login("john", "securePass");
    std::cout << "Test 5 - Valid login (john): " << result5 << std::endl;
    
    return 0;
}
