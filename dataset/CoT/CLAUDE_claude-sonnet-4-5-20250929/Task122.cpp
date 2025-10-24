
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <cstring>

class Task122 {
private:
    struct UserCredentials {
        std::string passwordHash;
        std::string salt;
    };
    
    std::map<std::string, UserCredentials> userDatabase;
    static const int SALT_LENGTH = 16;
    static const int ITERATIONS = 10000;
    
    std::string base64Encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
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
    
    std::string base64Decode(const std::string& input) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -8;
        
        for (unsigned char c : input) {
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
        
        unsigned char salt[SALT_LENGTH];
        for (int i = 0; i < SALT_LENGTH; i++) {
            salt[i] = dis(gen);
        }
        
        return base64Encode(salt, SALT_LENGTH);
    }
    
    std::string hashPassword(const std::string& password, const std::string& saltB64) {
        std::string saltDecoded = base64Decode(saltB64);
        std::string combined = saltDecoded + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        for (int i = 0; i < ITERATIONS; i++) {
            SHA256(hash, SHA256_DIGEST_LENGTH, hash);
        }
        
        return base64Encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    bool validatePassword(const std::string& password) {
        if (password.length() < 8) return false;
        
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (isupper(c)) hasUpper = true;
            else if (islower(c)) hasLower = true;
            else if (isdigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    std::string signupUser(const std::string& username, const std::string& password) {
        if (username.empty()) {
            return "Error: Username cannot be empty";
        }
        
        if (userDatabase.find(username) != userDatabase.end()) {
            return "Error: Username already exists";
        }
        
        if (!validatePassword(password)) {
            return "Error: Password must be at least 8 characters and contain uppercase, lowercase, digit, and special character";
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        
        userDatabase[username] = {passwordHash, salt};
        
        return "Success: User registered successfully";
    }
    
    bool verifyLogin(const std::string& username, const std::string& password) {
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        
        std::string hashedAttempt = hashPassword(password, it->second.salt);
        return hashedAttempt == it->second.passwordHash;
    }
};

int main() {
    Task122 authSystem;
    
    std::cout << "Test Case 1: Valid signup\\n";
    std::cout << authSystem.signupUser("john_doe", "SecurePass123!") << "\\n\\n";
    
    std::cout << "Test Case 2: Duplicate username\\n";
    std::cout << authSystem.signupUser("john_doe", "AnotherPass456@") << "\\n\\n";
    
    std::cout << "Test Case 3: Weak password\\n";
    std::cout << authSystem.signupUser("jane_doe", "weak") << "\\n\\n";
    
    std::cout << "Test Case 4: Valid signup\\n";
    std::cout << authSystem.signupUser("alice_smith", "MyP@ssw0rd!") << "\\n\\n";
    
    std::cout << "Test Case 5: Empty username\\n";
    std::cout << authSystem.signupUser("", "ValidPass123!") << "\\n\\n";
    
    std::cout << "Login Verification Test:\\n";
    std::cout << "Correct password: " << (authSystem.verifyLogin("john_doe", "SecurePass123!") ? "true" : "false") << "\\n";
    std::cout << "Wrong password: " << (authSystem.verifyLogin("john_doe", "WrongPass123!") ? "true" : "false") << "\\n";
    
    return 0;
}
