
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Simple user authentication system with secure credential handling
// Uses PBKDF2-HMAC-SHA256 for password hashing with unique salts

class SecureAuth {
private:
    static const size_t SALT_SIZE = 16;
    static const size_t HASH_SIZE = 32;
    static const int PBKDF2_ITERATIONS = 210000;
    
    struct UserCredential {
        std::vector<unsigned char> salt;
        std::vector<unsigned char> hash;
    };
    
    std::map<std::string, UserCredential> userDB;
    
    // Generate cryptographically secure random salt
    bool generateSalt(std::vector<unsigned char>& salt) {
        salt.resize(SALT_SIZE);
        // Use OpenSSL CSPRNG for secure randomness
        if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
            return false;
        }
        return true;
    }
    
    // Derive key from password using PBKDF2-HMAC-SHA256
    bool deriveKey(const std::string& password, 
                   const std::vector<unsigned char>& salt,
                   std::vector<unsigned char>& hash) {
        if (password.empty() || salt.size() != SALT_SIZE) {
            return false;
        }
        
        hash.resize(HASH_SIZE);
        
        // Use PBKDF2 with SHA256 for key derivation
        if (PKCS5_PBKDF2_HMAC(password.c_str(), 
                              static_cast<int>(password.length()),
                              salt.data(), 
                              static_cast<int>(salt.size()),
                              PBKDF2_ITERATIONS,
                              EVP_sha256(),
                              static_cast<int>(hash.size()),
                              hash.data()) != 1) {
            return false;
        }
        
        return true;
    }
    
    // Constant-time comparison to prevent timing attacks
    bool constantTimeCompare(const std::vector<unsigned char>& a,
                            const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) {
            return false;
        }
        
        // Use OpenSSL constant-time comparison
        return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
    }
    
    // Validate username format
    bool validateUsername(const std::string& username) {
        // Username must be 3-32 characters, alphanumeric and underscore only
        if (username.length() < 3 || username.length() > 32) {
            return false;
        }
        
        for (char c : username) {
            if (!((c >= 'a' && c <= 'z') || 
                  (c >= 'A' && c <= 'Z') || 
                  (c >= '0' && c <= '9') || 
                  c == '_')) {
                return false;
            }
        }
        return true;
    }
    
    // Validate password strength
    bool validatePassword(const std::string& password) {
        // Password must be 8-128 characters with complexity requirements
        if (password.length() < 8 || password.length() > 128) {
            return false;
        }
        
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        
        for (char c : password) {
            if (c >= 'A' && c <= 'Z') hasUpper = true;
            else if (c >= 'a' && c <= 'z') hasLower = true;
            else if (c >= '0' && c <= '9') hasDigit = true;
            else if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || 
                     (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) hasSpecial = true;
        }
        
        // Require at least 3 of 4 character types
        int complexity = (hasUpper ? 1 : 0) + (hasLower ? 1 : 0) + 
                        (hasDigit ? 1 : 0) + (hasSpecial ? 1 : 0);
        return complexity >= 3;
    }
    
public:
    // Register new user with secure password storage
    bool registerUser(const std::string& username, const std::string& password) {
        // Validate inputs (treat all inputs as untrusted)
        if (!validateUsername(username)) {
            std::cerr << "Invalid username format" << std::endl;
            return false;
        }
        
        if (!validatePassword(password)) {
            std::cerr << "Password does not meet complexity requirements" << std::endl;
            return false;
        }
        
        // Check if user already exists
        if (userDB.find(username) != userDB.end()) {
            std::cerr << "User already exists" << std::endl;
            return false;
        }
        
        UserCredential cred;
        
        // Generate unique salt for this user
        if (!generateSalt(cred.salt)) {
            std::cerr << "Failed to generate salt" << std::endl;
            return false;
        }
        
        // Derive password hash using PBKDF2
        if (!deriveKey(password, cred.salt, cred.hash)) {
            std::cerr << "Failed to derive key" << std::endl;
            return false;
        }
        
        // Store credentials
        userDB[username] = std::move(cred);
        
        // Never log or print passwords or hashes
        return true;
    }
    
    // Authenticate user with constant-time comparison
    bool authenticate(const std::string& username, const std::string& password) {
        // Validate inputs
        if (!validateUsername(username) || password.empty() || password.length() > 128) {
            return false;
        }
        
        // Check if user exists
        auto it = userDB.find(username);
        if (it == userDB.end()) {
            // Perform dummy operation to prevent timing attacks
            std::vector<unsigned char> dummySalt(SALT_SIZE);
            std::vector<unsigned char> dummyHash;
            RAND_bytes(dummySalt.data(), SALT_SIZE);
            deriveKey(password, dummySalt, dummyHash);
            return false;
        }
        
        // Derive hash from provided password
        std::vector<unsigned char> derivedHash;
        if (!deriveKey(password, it->second.salt, derivedHash)) {
            return false;
        }
        
        // Use constant-time comparison to prevent timing attacks
        bool result = constantTimeCompare(derivedHash, it->second.hash);
        
        // Securely clear sensitive data from memory
        std::fill(derivedHash.begin(), derivedHash.end(), 0);
        
        return result;
    }
    
    // Check if user exists
    bool userExists(const std::string& username) {
        if (!validateUsername(username)) {
            return false;
        }
        return userDB.find(username) != userDB.end();
    }
};

int main() {
    SecureAuth auth;
    
    // Test case 1: Register valid user
    std::cout << "Test 1 - Register user 'alice_01': ";
    bool result1 = auth.registerUser("alice_01", "SecurePass123!");
    std::cout << (result1 ? "PASS" : "FAIL") << std::endl;
    
    // Test case 2: Authenticate with correct password
    std::cout << "Test 2 - Authenticate alice_01 with correct password: ";
    bool result2 = auth.authenticate("alice_01", "SecurePass123!");
    std::cout << (result2 ? "PASS" : "FAIL") << std::endl;
    
    // Test case 3: Authenticate with incorrect password
    std::cout << "Test 3 - Authenticate alice_01 with wrong password: ";
    bool result3 = !auth.authenticate("alice_01", "WrongPass123!");
    std::cout << (result3 ? "PASS" : "FAIL") << std::endl;
    
    // Test case 4: Reject weak password
    std::cout << "Test 4 - Reject weak password '12345': ";
    bool result4 = !auth.registerUser("bob_user", "12345");
    std::cout << (result4 ? "PASS" : "FAIL") << std::endl;
    
    // Test case 5: Reject duplicate username
    std::cout << "Test 5 - Reject duplicate username: ";
    bool result5 = !auth.registerUser("alice_01", "AnotherPass456!");
    std::cout << (result5 ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
