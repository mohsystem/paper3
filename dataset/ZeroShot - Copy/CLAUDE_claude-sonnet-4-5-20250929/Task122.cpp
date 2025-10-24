
#include <iostream>
#include <map>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task122 {
private:
    struct UserCredentials {
        std::string salt;
        std::string hashedPassword;
    };
    
    static std::map<std::string, UserCredentials> userDatabase;
    
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
    static bool signup(const std::string& username, const std::string& password) {
        if (username.empty() || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.find(username) != userDatabase.end()) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        userDatabase[username] = {salt, hashedPassword};
        return true;
    }
    
    static bool verifyLogin(const std::string& username, const std::string& password) {
        if (userDatabase.find(username) == userDatabase.end()) {
            return false;
        }
        
        UserCredentials creds = userDatabase[username];
        std::string hashedAttempt = hashPassword(password, creds.salt);
        return hashedAttempt == creds.hashedPassword;
    }
};

std::map<std::string, Task122::UserCredentials> Task122::userDatabase;

int main() {
    std::cout << "Test Case 1: Valid signup" << std::endl;
    bool result1 = Task122::signup("user1", "SecurePass123");
    std::cout << "Signup successful: " << result1 << std::endl;
    std::cout << "Login verification: " << Task122::verifyLogin("user1", "SecurePass123") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Duplicate username" << std::endl;
    bool result2 = Task122::signup("user1", "AnotherPass456");
    std::cout << "Signup successful: " << result2 << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Weak password (too short)" << std::endl;
    bool result3 = Task122::signup("user2", "short");
    std::cout << "Signup successful: " << result3 << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Multiple valid users" << std::endl;
    bool result4a = Task122::signup("alice", "AlicePass2024");
    bool result4b = Task122::signup("bob", "BobSecure789");
    std::cout << "Alice signup: " << result4a << ", Bob signup: " << result4b << std::endl;
    std::cout << "Alice login: " << Task122::verifyLogin("alice", "AlicePass2024") << std::endl;
    std::cout << "Bob login: " << Task122::verifyLogin("bob", "BobSecure789") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Invalid login attempt" << std::endl;
    Task122::signup("user3", "ValidPassword123");
    bool validLogin = Task122::verifyLogin("user3", "ValidPassword123");
    bool invalidLogin = Task122::verifyLogin("user3", "WrongPassword");
    std::cout << "Valid login: " << validLogin << std::endl;
    std::cout << "Invalid login: " << invalidLogin << std::endl;
    
    return 0;
}
