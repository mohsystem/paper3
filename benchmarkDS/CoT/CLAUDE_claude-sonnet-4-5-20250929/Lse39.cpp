
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <cstring>

class Lse39 {
private:
    std::map<std::string, std::string> users;
    std::map<std::string, std::string> salts;
    
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
    
    std::string base64Encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string ret;
        int i = 0;
        unsigned char array3[3];
        unsigned char array4[4];
        
        while (length--) {
            array3[i++] = *(buffer++);
            if (i == 3) {
                array4[0] = (array3[0] & 0xfc) >> 2;
                array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
                array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
                array4[3] = array3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    ret += base64_chars[array4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (int j = i; j < 3; j++)
                array3[j] = '\\0';
            
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            
            for (int j = 0; j < i + 1; j++)
                ret += base64_chars[array4[j]];
            
            while (i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt.c_str(), salt.length());
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) return false;
        
        volatile int result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
public:
    Lse39() {
        registerUser("user1", "Password123!");
        registerUser("admin", "Admin@2024");
        registerUser("test", "Test#Pass1");
        registerUser("john", "John$ecure9");
        registerUser("alice", "Alice&Pass7");
    }
    
    void registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty() || password.length() < 8) {
            throw std::invalid_argument("Invalid username or password");
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        users[username] = hashedPassword;
        salts[username] = salt;
    }
    
    std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "Username or password is incorrect";
        }
        
        if (users.find(username) == users.end()) {
            return "Username or password is incorrect";
        }
        
        try {
            std::string storedHash = users[username];
            std::string salt = salts[username];
            std::string providedHash = hashPassword(password, salt);
            
            if (constantTimeCompare(storedHash, providedHash)) {
                return "Login successful! Redirecting to session page...";
            } else {
                return "Username or password is incorrect";
            }
        } catch (...) {
            return "An error occurred during authentication";
        }
    }
};

int main() {
    std::cout << "=== Login System Test Cases ===" << std::endl << std::endl;
    
    Lse39 authSystem;
    
    std::cout << "Test 1 - Valid credentials (user1):" << std::endl;
    std::cout << authSystem.login("user1", "Password123!") << std::endl << std::endl;
    
    std::cout << "Test 2 - Valid credentials (admin):" << std::endl;
    std::cout << authSystem.login("admin", "Admin@2024") << std::endl << std::endl;
    
    std::cout << "Test 3 - Invalid password:" << std::endl;
    std::cout << authSystem.login("user1", "WrongPassword") << std::endl << std::endl;
    
    std::cout << "Test 4 - Non-existent user:" << std::endl;
    std::cout << authSystem.login("nonexistent", "Password123!") << std::endl << std::endl;
    
    std::cout << "Test 5 - Empty credentials:" << std::endl;
    std::cout << authSystem.login("", "") << std::endl << std::endl;
    
    return 0;
}
