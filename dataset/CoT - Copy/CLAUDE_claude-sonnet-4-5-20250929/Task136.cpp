
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <regex>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>

class Task136 {
private:
    static const int SALT_LENGTH = 16;
    static const int MAX_LOGIN_ATTEMPTS = 3;
    
    struct UserCredential {
        std::string username;
        std::string hashedPassword;
        std::string salt;
    };
    
    std::map<std::string, UserCredential> userDatabase;
    std::map<std::string, int> loginAttempts;
    
    std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char* base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int i = 0;
        unsigned char array3[3];
        unsigned char array4[4];
        
        for (unsigned char c : data) {
            array3[i++] = c;
            if (i == 3) {
                array4[0] = (array3[0] & 0xfc) >> 2;
                array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
                array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
                array4[3] = array3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    result += base64Chars[array4[i]];
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
                result += base64Chars[array4[j]];
            
            while (i++ < 3)
                result += '=';
        }
        
        return result;
    }
    
    std::vector<unsigned char> base64Decode(const std::string& encoded) {
        static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> result;
        int i = 0;
        unsigned char array4[4], array3[3];
        
        for (char c : encoded) {
            if (c == '=') break;
            if (base64Chars.find(c) == std::string::npos) continue;
            
            array4[i++] = c;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    array4[i] = base64Chars.find(array4[i]);
                
                array3[0] = (array4[0] << 2) + ((array4[1] & 0x30) >> 4);
                array3[1] = ((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2);
                array3[2] = ((array4[2] & 0x3) << 6) + array4[3];
                
                for (i = 0; i < 3; i++)
                    result.push_back(array3[i]);
                i = 0;
            }
        }
        
        return result;
    }
    
public:
    Task136() {}
    
    std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::vector<unsigned char> salt(SALT_LENGTH);
        for (int i = 0; i < SALT_LENGTH; i++) {
            salt[i] = dis(gen);
        }
        
        return base64Encode(salt);
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::vector<unsigned char> saltBytes = base64Decode(salt);
        unsigned char hash[SHA256_DIGEST_LENGTH];
        
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          saltBytes.data(), saltBytes.size(),
                          100000, EVP_sha256(),
                          SHA256_DIGEST_LENGTH, hash);
        
        std::vector<unsigned char> hashVec(hash, hash + SHA256_DIGEST_LENGTH);
        return base64Encode(hashVec);
    }
    
    bool validateUsername(const std::string& username) {
        if (username.empty()) return false;
        std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
        return std::regex_match(username, pattern);
    }
    
    bool validatePassword(const std::string& password) {
        if (password.empty()) return false;
        return password.length() >= 8;
    }
    
    std::string registerUser(const std::string& username, const std::string& password) {
        if (!validateUsername(username)) {
            return "Invalid username. Must be 3-20 alphanumeric characters or underscore.";
        }
        
        if (!validatePassword(password)) {
            return "Invalid password. Must be at least 8 characters long.";
        }
        
        if (userDatabase.find(username) != userDatabase.end()) {
            return "Username already exists.";
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        
        UserCredential cred = {username, hashedPassword, salt};
        userDatabase[username] = cred;
        loginAttempts[username] = 0;
        
        return "User registered successfully.";
    }
    
    bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) return false;
        
        int result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        
        return result == 0;
    }
    
    std::string authenticateUser(const std::string& username, const std::string& password) {
        if (!validateUsername(username)) {
            return "Authentication failed.";
        }
        
        if (loginAttempts[username] >= MAX_LOGIN_ATTEMPTS) {
            return "Account locked due to too many failed attempts.";
        }
        
        auto it = userDatabase.find(username);
        
        if (it == userDatabase.end()) {
            hashPassword(password, generateSalt());
            return "Authentication failed.";
        }
        
        std::string hashedInputPassword = hashPassword(password, it->second.salt);
        
        if (constantTimeEquals(hashedInputPassword, it->second.hashedPassword)) {
            loginAttempts[username] = 0;
            return "Authentication successful.";
        } else {
            loginAttempts[username]++;
            
            if (loginAttempts[username] >= MAX_LOGIN_ATTEMPTS) {
                return "Account locked due to too many failed attempts.";
            }
            
            return "Authentication failed.";
        }
    }
    
    std::string resetLoginAttempts(const std::string& username) {
        if (userDatabase.find(username) != userDatabase.end()) {
            loginAttempts[username] = 0;
            return "Login attempts reset successfully.";
        }
        return "User not found.";
    }
};

int main() {
    Task136 auth;
    
    std::cout << "=== Test Case 1: Valid User Registration ===" << std::endl;
    std::cout << auth.registerUser("john_doe", "SecurePass123") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Invalid Username (too short) ===" << std::endl;
    std::cout << auth.registerUser("ab", "SecurePass123") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Invalid Password (too short) ===" << std::endl;
    std::cout << auth.registerUser("jane_doe", "short") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Successful Authentication ===" << std::endl;
    std::cout << auth.authenticateUser("john_doe", "SecurePass123") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Failed Authentication and Account Lockout ===" << std::endl;
    std::cout << auth.authenticateUser("john_doe", "WrongPass1") << std::endl;
    std::cout << auth.authenticateUser("john_doe", "WrongPass2") << std::endl;
    std::cout << auth.authenticateUser("john_doe", "WrongPass3") << std::endl;
    std::cout << auth.authenticateUser("john_doe", "SecurePass123") << std::endl;
    std::cout << auth.resetLoginAttempts("john_doe") << std::endl;
    std::cout << auth.authenticateUser("john_doe", "SecurePass123") << std::endl;
    
    return 0;
}
