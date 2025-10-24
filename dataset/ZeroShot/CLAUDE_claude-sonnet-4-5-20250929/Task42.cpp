
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

class Task42 {
private:
    struct UserCredentials {
        std::string passwordHash;
        std::string salt;
    };
    
    std::map<std::string, UserCredentials> userDatabase;
    
    std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, sizeof(salt));
        
        std::stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
        }
        return ss.str();
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string saltedPassword = salt + password;
        
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.length());
        SHA256_Final(hash, &sha256);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    Task42() {}
    
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.find(username) != userDatabase.end()) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        
        UserCredentials credentials;
        credentials.passwordHash = passwordHash;
        credentials.salt = salt;
        
        userDatabase[username] = credentials;
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
        
        std::string hashedInput = hashPassword(password, it->second.salt);
        return hashedInput == it->second.passwordHash;
    }
    
    bool changePassword(const std::string& username, const std::string& oldPassword, 
                       const std::string& newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        if (newPassword.length() < 8) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(newPassword, salt);
        
        userDatabase[username].passwordHash = passwordHash;
        userDatabase[username].salt = salt;
        return true;
    }
};

int main() {
    Task42 authSystem;
    
    // Test Case 1: Register new user
    std::cout << "Test 1 - Register user 'alice': " 
              << (authSystem.registerUser("alice", "SecurePass123") ? "true" : "false") << std::endl;
    
    // Test Case 2: Authenticate valid user
    std::cout << "Test 2 - Login alice with correct password: " 
              << (authSystem.authenticateUser("alice", "SecurePass123") ? "true" : "false") << std::endl;
    
    // Test Case 3: Authenticate with wrong password
    std::cout << "Test 3 - Login alice with wrong password: " 
              << (authSystem.authenticateUser("alice", "WrongPass123") ? "true" : "false") << std::endl;
    
    // Test Case 4: Register duplicate username
    std::cout << "Test 4 - Register duplicate user 'alice': " 
              << (authSystem.registerUser("alice", "AnotherPass123") ? "true" : "false") << std::endl;
    
    // Test Case 5: Change password
    authSystem.registerUser("bob", "OldPassword123");
    std::cout << "Test 5 - Change bob's password: " 
              << (authSystem.changePassword("bob", "OldPassword123", "NewPassword123") ? "true" : "false") << std::endl;
    std::cout << "Test 5 - Login bob with new password: " 
              << (authSystem.authenticateUser("bob", "NewPassword123") ? "true" : "false") << std::endl;
    
    return 0;
}
