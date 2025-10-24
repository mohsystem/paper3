
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <cstring>

class Task42 {
private:
    static const int SALT_LENGTH = 16;
    static const int MAX_LOGIN_ATTEMPTS = 3;
    
    struct UserCredentials {
        std::string passwordHash;
        std::string salt;
        
        UserCredentials() {}
        UserCredentials(const std::string& hash, const std::string& s) 
            : passwordHash(hash), salt(s) {}
    };
    
    std::map<std::string, UserCredentials> userDatabase;
    std::map<std::string, int> loginAttempts;
    
    std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        unsigned char salt[SALT_LENGTH];
        for (int i = 0; i < SALT_LENGTH; i++) {
            salt[i] = static_cast<unsigned char>(dis(gen));
        }
        
        return base64Encode(salt, SALT_LENGTH);
    }
    
    std::string base64Encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        while (length--) {
            char_array_3[i++] = *(buffer++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while(i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
    std::string base64Decode(const std::string& encoded_string) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;
        
        while (in_len-- && (encoded_string[in_] != '=') && 
               (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;
            
            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            
            for (j = 0; j < i - 1; j++)
                ret += char_array_3[j];
        }
        
        return ret;
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string saltDecoded = base64Decode(salt);
        std::string combined = saltDecoded + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), 
               combined.length(), hash);
        
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
public:
    Task42() {}
    
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty() || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.find(username) != userDatabase.end()) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        userDatabase[username] = UserCredentials(passwordHash, salt);
        loginAttempts[username] = 0;
        return true;
    }
    
    bool authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        if (userDatabase.find(username) == userDatabase.end()) {
            return false;
        }
        
        int attempts = loginAttempts[username];
        if (attempts >= MAX_LOGIN_ATTEMPTS) {
            return false;
        }
        
        UserCredentials credentials = userDatabase[username];
        std::string passwordHash = hashPassword(password, credentials.salt);
        
        if (passwordHash == credentials.passwordHash) {
            loginAttempts[username] = 0;
            return true;
        } else {
            loginAttempts[username] = attempts + 1;
            return false;
        }
    }
    
    bool changePassword(const std::string& username, const std::string& oldPassword, 
                       const std::string& newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        if (newPassword.empty() || newPassword.length() < 8) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(newPassword, salt);
        userDatabase[username] = UserCredentials(passwordHash, salt);
        return true;
    }
    
    void resetLoginAttempts(const std::string& username) {
        if (userDatabase.find(username) != userDatabase.end()) {
            loginAttempts[username] = 0;
        }
    }
};

int main() {
    Task42 authSystem;
    
    std::cout << "Test Case 1: User Registration" << std::endl;
    bool result1 = authSystem.registerUser("alice", "SecurePass123");
    std::cout << "Register alice: " << (result1 ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "\\nTest Case 2: Successful Authentication" << std::endl;
    bool result2 = authSystem.authenticateUser("alice", "SecurePass123");
    std::cout << "Authenticate alice: " << (result2 ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "\\nTest Case 3: Failed Authentication (wrong password)" << std::endl;
    bool result3 = authSystem.authenticateUser("alice", "WrongPass123");
    std::cout << "Authenticate with wrong password: " << (result3 ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "\\nTest Case 4: Password Change" << std::endl;
    bool result4 = authSystem.changePassword("alice", "SecurePass123", "NewSecure456");
    std::cout << "Change password: " << (result4 ? "SUCCESS" : "FAILED") << std::endl;
    bool result4b = authSystem.authenticateUser("alice", "NewSecure456");
    std::cout << "Authenticate with new password: " << (result4b ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "\\nTest Case 5: Account Lockout after multiple failed attempts" << std::endl;
    authSystem.registerUser("bob", "BobPass123");
    authSystem.authenticateUser("bob", "wrong1");
    authSystem.authenticateUser("bob", "wrong2");
    authSystem.authenticateUser("bob", "wrong3");
    bool result5 = authSystem.authenticateUser("bob", "BobPass123");
    std::cout << "Authenticate after 3 failed attempts: " << (result5 ? "SUCCESS" : "FAILED (LOCKED)") << std::endl;
    
    return 0;
}
