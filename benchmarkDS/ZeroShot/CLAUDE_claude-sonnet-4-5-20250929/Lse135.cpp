
#include <iostream>
#include <string>
#include <map>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Lse135 {
private:
    static std::map<std::string, std::string> userDatabase;
    
    // Hash password using SHA-256
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    // Initialize the user database
    static void initializeDatabase() {
        userDatabase["admin"] = hashPassword("admin123");
        userDatabase["user1"] = hashPassword("user456");
        userDatabase["testuser"] = hashPassword("test789");
    }
    
    // Check credentials
    static bool checkMod(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        
        std::string inputHash = hashPassword(password);
        return it->second == inputHash;
    }
};

// Initialize static member
std::map<std::string, std::string> Lse135::userDatabase;

int main() {
    Lse135::initializeDatabase();
    
    std::cout << "Test Case 1 (Valid - admin): " << std::boolalpha << Lse135::checkMod("admin", "admin123") << std::endl;
    std::cout << "Test Case 2 (Valid - user1): " << std::boolalpha << Lse135::checkMod("user1", "user456") << std::endl;
    std::cout << "Test Case 3 (Invalid password): " << std::boolalpha << Lse135::checkMod("admin", "wrongpass") << std::endl;
    std::cout << "Test Case 4 (Invalid username): " << std::boolalpha << Lse135::checkMod("nonexistent", "admin123") << std::endl;
    std::cout << "Test Case 5 (Empty credentials): " << std::boolalpha << Lse135::checkMod("", "") << std::endl;
    
    return 0;
}
