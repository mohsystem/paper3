
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task42 {
private:
    struct UserCredential {
        std::string salt;
        std::string hashedPassword;
        
        UserCredential() {}
        UserCredential(const std::string& s, const std::string& h) 
            : salt(s), hashedPassword(h) {}
    };
    
    std::map<std::string, UserCredential> users;
    
    std::string base64_encode(const unsigned char* data, size_t len) {
        const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }
    
    std::string base64_decode(const std::string& input) {
        const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -8;
        
        for (char c : input) {
            if (c == '=') break;
            size_t pos = base64_chars.find(c);
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
        
        return base64_encode(salt, 16);
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string saltDecoded = base64_decode(salt);
        std::string combined = saltDecoded + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    bool registerUser(const std::string& username, const std::string& password) {
        if (users.find(username) != users.end()) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        users[username] = UserCredential(salt, hashedPassword);
        return true;
    }
    
    bool authenticateUser(const std::string& username, const std::string& password) {
        if (users.find(username) == users.end()) {
            return false;
        }
        
        UserCredential& credential = users[username];
        std::string hashedPassword = hashPassword(password, credential.salt);
        return hashedPassword == credential.hashedPassword;
    }
    
    bool deleteUser(const std::string& username) {
        if (users.find(username) == users.end()) {
            return false;
        }
        users.erase(username);
        return true;
    }
    
    bool changePassword(const std::string& username, const std::string& oldPassword, 
                       const std::string& newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(newPassword, salt);
        users[username] = UserCredential(salt, hashedPassword);
        return true;
    }
};

int main() {
    Task42 authSystem;
    
    // Test Case 1: Register new user
    std::cout << "Test 1 - Register user 'alice':" << std::endl;
    bool result1 = authSystem.registerUser("alice", "password123");
    std::cout << "Registration successful: " << (result1 ? "true" : "false") << std::endl;
    
    // Test Case 2: Authenticate with correct password
    std::cout << "\\nTest 2 - Authenticate alice with correct password:" << std::endl;
    bool result2 = authSystem.authenticateUser("alice", "password123");
    std::cout << "Authentication successful: " << (result2 ? "true" : "false") << std::endl;
    
    // Test Case 3: Authenticate with incorrect password
    std::cout << "\\nTest 3 - Authenticate alice with incorrect password:" << std::endl;
    bool result3 = authSystem.authenticateUser("alice", "wrongpassword");
    std::cout << "Authentication successful: " << (result3 ? "true" : "false") << std::endl;
    
    // Test Case 4: Change password
    std::cout << "\\nTest 4 - Change password for alice:" << std::endl;
    bool result4 = authSystem.changePassword("alice", "password123", "newpassword456");
    std::cout << "Password change successful: " << (result4 ? "true" : "false") << std::endl;
    bool result4b = authSystem.authenticateUser("alice", "newpassword456");
    std::cout << "Authentication with new password: " << (result4b ? "true" : "false") << std::endl;
    
    // Test Case 5: Register duplicate user and delete user
    std::cout << "\\nTest 5 - Register duplicate user and delete:" << std::endl;
    bool result5a = authSystem.registerUser("alice", "anotherpassword");
    std::cout << "Duplicate registration blocked: " << (!result5a ? "true" : "false") << std::endl;
    bool result5b = authSystem.deleteUser("alice");
    std::cout << "User deletion successful: " << (result5b ? "true" : "false") << std::endl;
    bool result5c = authSystem.authenticateUser("alice", "newpassword456");
    std::cout << "Authentication after deletion: " << (result5c ? "true" : "false") << std::endl;
    
    return 0;
}
