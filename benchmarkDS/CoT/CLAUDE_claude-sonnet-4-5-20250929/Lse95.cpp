
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

class Lse95 {
private:
    static const int ITERATIONS = 100000;
    static const int SALT_LENGTH = 16;
    static const int KEY_LENGTH = 32;
    
    static std::string base64_encode(const std::vector<unsigned char>& data) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (unsigned char c : data) {
            val = (val << 8) + c;
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
        
        for (char c : input) {
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
    
    static std::vector<unsigned char> generateSalt() {
        std::vector<unsigned char> salt(SALT_LENGTH);
        RAND_bytes(salt.data(), SALT_LENGTH);
        return salt;
    }
    
    static std::vector<unsigned char> hashPassword(const std::string& password, 
                                                   const std::vector<unsigned char>& salt) {
        std::vector<unsigned char> hash(KEY_LENGTH);
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          ITERATIONS, EVP_sha256(),
                          KEY_LENGTH, hash.data());
        return hash;
    }
    
    static bool slowEquals(const std::vector<unsigned char>& a, 
                          const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        unsigned char result = 0;
        for (size_t i = 0; i < a.size(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
public:
    static std::pair<std::string, std::string> registerUser(const std::string& username, 
                                                            const std::string& password) {
        auto salt = generateSalt();
        auto hash = hashPassword(password, salt);
        
        std::string saltBase64 = base64_encode(salt);
        std::string hashBase64 = base64_encode(hash);
        std::string storedPassword = saltBase64 + ":" + hashBase64;
        
        return {username, storedPassword};
    }
    
    static bool verifyPassword(const std::string& password, const std::string& storedPassword) {
        size_t colonPos = storedPassword.find(':');
        if (colonPos == std::string::npos) return false;
        
        std::string saltBase64 = storedPassword.substr(0, colonPos);
        std::string hashBase64 = storedPassword.substr(colonPos + 1);
        
        auto salt = base64_decode(saltBase64);
        auto storedHash = base64_decode(hashBase64);
        auto computedHash = hashPassword(password, salt);
        
        return slowEquals(storedHash, computedHash);
    }
};

int main() {
    // Test Case 1: Basic registration
    auto [user1, pass1] = Lse95::registerUser("alice", "SecurePass123!");
    std::cout << "Test 1 - Registered: " << user1 << std::endl;
    std::cout << "Verification: " << (Lse95::verifyPassword("SecurePass123!", pass1) ? "true" : "false") << std::endl;
    
    // Test Case 2: Different user
    auto [user2, pass2] = Lse95::registerUser("bob", "MyPassword456@");
    std::cout << "\\nTest 2 - Registered: " << user2 << std::endl;
    std::cout << "Verification: " << (Lse95::verifyPassword("MyPassword456@", pass2) ? "true" : "false") << std::endl;
    
    // Test Case 3: Wrong password
    std::cout << "\\nTest 3 - Wrong password: " << (Lse95::verifyPassword("WrongPass", pass1) ? "true" : "false") << std::endl;
    
    // Test Case 4: Complex password
    auto [user3, pass3] = Lse95::registerUser("charlie", "C0mpl3x!P@ssw0rd#2024");
    std::cout << "\\nTest 4 - Registered: " << user3 << std::endl;
    std::cout << "Verification: " << (Lse95::verifyPassword("C0mpl3x!P@ssw0rd#2024", pass3) ? "true" : "false") << std::endl;
    
    // Test Case 5: Another user
    auto [user4, pass4] = Lse95::registerUser("david", "AnotherSecure987$");
    std::cout << "\\nTest 5 - Registered: " << user4 << std::endl;
    std::cout << "Verification: " << (Lse95::verifyPassword("AnotherSecure987$", pass4) ? "true" : "false") << std::endl;
    
    return 0;
}
