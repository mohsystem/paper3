
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/rand.h>

class Lse136 {
private:
    static std::map<std::string, std::vector<unsigned char>> userDatabase;
    static std::map<std::string, std::vector<unsigned char>> saltDatabase;
    
    static std::vector<unsigned char> generateSalt() {
        std::vector<unsigned char> salt(16);
        RAND_bytes(salt.data(), 16);
        return salt;
    }
    
    static std::vector<unsigned char> hashPassword(const std::string& password, 
                                                   const std::vector<unsigned char>& salt) {
        std::vector<unsigned char> combined;
        combined.insert(combined.end(), salt.begin(), salt.end());
        combined.insert(combined.end(), password.begin(), password.end());
        
        std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
        SHA256(combined.data(), combined.size(), hash.data());
        return hash;
    }
    
    static bool constantTimeEquals(const std::vector<unsigned char>& a, 
                                   const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) {
            return false;
        }
        
        unsigned char result = 0;
        for (size_t i = 0; i < a.size(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
public:
    static void addUser(const std::string& username, const std::string& password) {
        std::vector<unsigned char> salt = generateSalt();
        std::vector<unsigned char> hashedPassword = hashPassword(password, salt);
        userDatabase[username] = hashedPassword;
        saltDatabase[username] = salt;
    }
    
    static std::string check_mod(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "false";
        }
        
        // Trim username
        std::string trimmedUsername = username;
        trimmedUsername.erase(0, trimmedUsername.find_first_not_of(" \\t\\n\\r"));
        trimmedUsername.erase(trimmedUsername.find_last_not_of(" \\t\\n\\r") + 1);
        
        if (userDatabase.find(trimmedUsername) == userDatabase.end()) {
            return "false";
        }
        
        std::vector<unsigned char> salt = saltDatabase[trimmedUsername];
        std::vector<unsigned char> hashedPassword = hashPassword(password, salt);
        std::vector<unsigned char> storedHash = userDatabase[trimmedUsername];
        
        if (constantTimeEquals(hashedPassword, storedHash)) {
            return "true";
        }
        
        return "false";
    }
    
    static void initializeUsers() {
        addUser("admin", "Admin@123");
        addUser("user1", "Pass@word1");
        addUser("testuser", "Test@123");
    }
};

std::map<std::string, std::vector<unsigned char>> Lse136::userDatabase;
std::map<std::string, std::vector<unsigned char>> Lse136::saltDatabase;

int main() {
    Lse136::initializeUsers();
    
    // Test cases
    std::cout << "Test Case 1 (Valid admin): " << Lse136::check_mod("admin", "Admin@123") << std::endl;
    std::cout << "Test Case 2 (Valid user1): " << Lse136::check_mod("user1", "Pass@word1") << std::endl;
    std::cout << "Test Case 3 (Invalid password): " << Lse136::check_mod("admin", "wrongpass") << std::endl;
    std::cout << "Test Case 4 (Non-existent user): " << Lse136::check_mod("hacker", "password") << std::endl;
    std::cout << "Test Case 5 (Empty credentials): " << Lse136::check_mod("", "") << std::endl;
    
    return 0;
}
