
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>

class Lse54 {
private:
    static std::map<std::string, std::string> userDatabase;
    static std::map<std::string, std::string> salts;
    
    static std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        return ss.str();
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    static void initialize() {
        std::string salt = generateSalt();
        salts["admin"] = salt;
        userDatabase["admin"] = hashPassword("admin123", salt);
    }
    
    static std::string login() {
        return "<!DOCTYPE html>\\n"
               "<html>\\n"
               "<head><title>Login</title></head>\\n"
               "<body>\\n"
               "<h2>Login Form</h2>\\n"
               "<form method='POST' action='/do_login'>\\n"
               "  <label>Username: <input type='text' name='username' required pattern='[a-zA-Z0-9_]{3,20}'></label><br>\\n"
               "  <label>Password: <input type='password' name='password' required minlength='6'></label><br>\\n"
               "  <input type='submit' value='Login'>\\n"
               "</form>\\n"
               "</body>\\n"
               "</html>";
    }
    
    static std::string doLogin(const std::string& username, const std::string& password) {
        // Input validation
        if (username.empty() || password.empty()) {
            return "Invalid credentials";
        }
        
        // Sanitize username
        std::regex usernamePattern("^[a-zA-Z0-9_]{3,20}$");
        if (!std::regex_match(username, usernamePattern)) {
            return "Invalid username format";
        }
        
        // Check if user exists and verify password
        auto it = userDatabase.find(username);
        if (it != userDatabase.end()) {
            std::string salt = salts[username];
            std::string hashedPassword = hashPassword(password, salt);
            
            if (hashedPassword == it->second) {
                if (username == "admin") {
                    return "REDIRECT:/admin_page";
                }
                return "Login successful";
            }
        }
        
        return "Invalid credentials";
    }
};

std::map<std::string, std::string> Lse54::userDatabase;
std::map<std::string, std::string> Lse54::salts;

int main() {
    Lse54::initialize();
    
    std::cout << "Test Case 1 - GET login form:" << std::endl;
    std::cout << Lse54::login().substr(0, 50) << "...\\n" << std::endl;
    
    std::cout << "Test Case 2 - Valid admin login:" << std::endl;
    std::cout << Lse54::doLogin("admin", "admin123") << "\\n" << std::endl;
    
    std::cout << "Test Case 3 - Invalid password:" << std::endl;
    std::cout << Lse54::doLogin("admin", "wrongpass") << "\\n" << std::endl;
    
    std::cout << "Test Case 4 - Non-existent user:" << std::endl;
    std::cout << Lse54::doLogin("user123", "password") << "\\n" << std::endl;
    
    std::cout << "Test Case 5 - Empty credentials:" << std::endl;
    std::cout << Lse54::doLogin("", "") << "\\n" << std::endl;
    
    return 0;
}
