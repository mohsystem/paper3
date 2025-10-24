
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task136 {
private:
    struct UserCredentials {
        std::string salt;
        std::string hashedPassword;
    };
    
    std::map<std::string, UserCredentials> userDatabase;
    
    std::string base64Encode(const unsigned char* buffer, size_t length) {
        static const char* base64Chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (size_t i = 0; i < length; i++) {
            val = (val << 8) + buffer[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64Chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }
    
    std::string base64Decode(const std::string& input) {
        static const std::string base64Chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -8;
        
        for (unsigned char c : input) {
            if (c == '=') break;
            size_t pos = base64Chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + pos;
            valb += 6;
            if (valb >= 0) {
                result.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }
    
public:
    std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        unsigned char salt[16];
        for (int i = 0; i < 16; i++) {
            salt[i] = dis(gen);
        }
        return base64Encode(salt, 16);
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string saltDecoded = base64Decode(salt);
        std::string combined = saltDecoded + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty() || password.length() < 8) {
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
    
    bool authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        
        std::string hashedPassword = hashPassword(password, it->second.salt);
        return hashedPassword == it->second.hashedPassword;
    }
};

int main() {
    Task136 authSystem;
    
    // Test Case 1: Successful registration and authentication
    std::cout << "Test 1 - Register and authenticate user:" << std::endl;
    bool registered = authSystem.registerUser("alice", "SecurePass123");
    std::cout << "Registration: " << (registered ? "true" : "false") << std::endl;
    bool authenticated = authSystem.authenticateUser("alice", "SecurePass123");
    std::cout << "Authentication: " << (authenticated ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Failed authentication with wrong password
    std::cout << "Test 2 - Wrong password:" << std::endl;
    bool wrongPass = authSystem.authenticateUser("alice", "WrongPassword");
    std::cout << "Authentication: " << (wrongPass ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Registration with weak password
    std::cout << "Test 3 - Weak password:" << std::endl;
    bool weakPass = authSystem.registerUser("bob", "weak");
    std::cout << "Registration: " << (weakPass ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Duplicate username registration
    std::cout << "Test 4 - Duplicate username:" << std::endl;
    bool duplicate = authSystem.registerUser("alice", "AnotherPass456");
    std::cout << "Registration: " << (duplicate ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Non-existent user authentication
    std::cout << "Test 5 - Non-existent user:" << std::endl;
    bool nonExistent = authSystem.authenticateUser("charlie", "SomePassword789");
    std::cout << "Authentication: " << (nonExistent ? "true" : "false") << std::endl;
    
    return 0;
}
