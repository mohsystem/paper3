
#include <iostream>
#include <string>
#include <map>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>

class Lse96 {
private:
    static const int SALT_LENGTH = 16;
    static const int ITERATIONS = 100000;
    static const int KEY_LENGTH = 32;
    static std::map<std::string, std::string> database;
    
    static std::string base64_encode(const unsigned char* data, size_t len) {
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
    
    static std::vector<unsigned char> base64_decode(const std::string& input) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> result;
        int val = 0, valb = -8;
        
        for (unsigned char c : input) {
            if (c == '=') break;
            size_t pos = base64_chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + pos;
            valb += 6;
            if (valb >= 0) {
                result.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }
        return result;
    }
    
public:
    static std::string generateSalt() {
        unsigned char salt[SALT_LENGTH];
        if (RAND_bytes(salt, SALT_LENGTH) != 1) {
            return "";
        }
        return base64_encode(salt, SALT_LENGTH);
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::vector<unsigned char> saltBytes = base64_decode(salt);
        unsigned char hash[KEY_LENGTH];
        
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                              saltBytes.data(), saltBytes.size(),
                              ITERATIONS, EVP_sha256(),
                              KEY_LENGTH, hash) != 1) {
            return "";
        }
        
        return base64_encode(hash, KEY_LENGTH);
    }
    
    static bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.length() < 8) {
            return false;
        }
        
        std::string salt = generateSalt();
        if (salt.empty()) {
            return false;
        }
        
        std::string hashedPassword = hashPassword(password, salt);
        if (hashedPassword.empty()) {
            return false;
        }
        
        std::string storedValue = salt + ":" + hashedPassword;
        database[username] = storedValue;
        return true;
    }
    
    static bool verifyUser(const std::string& username, const std::string& password) {
        auto it = database.find(username);
        if (it == database.end()) {
            return false;
        }
        
        std::string storedValue = it->second;
        size_t colonPos = storedValue.find(':');
        if (colonPos == std::string::npos) {
            return false;
        }
        
        std::string salt = storedValue.substr(0, colonPos);
        std::string storedHash = storedValue.substr(colonPos + 1);
        std::string computedHash = hashPassword(password, salt);
        
        return storedHash == computedHash;
    }
};

std::map<std::string, std::string> Lse96::database;

int main() {
    // Test Case 1: Valid registration
    std::cout << "Test 1 - Valid registration: " 
              << Lse96::registerUser("user1", "SecurePass123!") << std::endl;
    
    // Test Case 2: Valid registration and verification
    Lse96::registerUser("user2", "MyP@ssw0rd");
    std::cout << "Test 2 - Valid login: " 
              << Lse96::verifyUser("user2", "MyP@ssw0rd") << std::endl;
    
    // Test Case 3: Invalid password (too short)
    std::cout << "Test 3 - Short password: " 
              << Lse96::registerUser("user3", "short") << std::endl;
    
    // Test Case 4: Invalid login
    Lse96::registerUser("user4", "ValidPass123");
    std::cout << "Test 4 - Wrong password: " 
              << Lse96::verifyUser("user4", "WrongPass123") << std::endl;
    
    // Test Case 5: Empty username
    std::cout << "Test 5 - Empty username: " 
              << Lse96::registerUser("", "ValidPass123") << std::endl;
    
    return 0;
}
