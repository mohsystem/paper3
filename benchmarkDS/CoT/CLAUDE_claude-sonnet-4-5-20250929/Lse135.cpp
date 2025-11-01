
#include <iostream>
#include <string>
#include <unordered_map>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>

class Lse135 {
private:
    static std::unordered_map<std::string, std::string> credentials;
    
    // Secure password hashing using SHA-256
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    // Constant-time string comparison to prevent timing attacks
    static bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    // Sanitize username
    static std::string sanitizeUsername(const std::string& username) {
        std::string sanitized;
        for (char c : username) {
            if (std::isalnum(c) || c == '_') {
                sanitized += c;
            }
        }
        return sanitized;
    }
    
    // Trim whitespace
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }

public:
    // Initialize credentials
    static void initializeCredentials() {
        credentials["admin"] = hashPassword("Admin@123");
        credentials["user1"] = hashPassword("User@456");
        credentials["moderator"] = hashPassword("Mod@789");
    }
    
    // Secure credential validation
    static bool check_mod(const std::string& username, const std::string& password) {
        // Input validation
        if (username.empty() || password.empty()) {
            return false;
        }
        
        std::string cleanUsername = trim(username);
        std::string cleanPassword = trim(password);
        
        if (cleanUsername.empty() || cleanPassword.empty()) {
            return false;
        }
        
        // Sanitize username
        cleanUsername = sanitizeUsername(cleanUsername);
        
        // Check if username exists
        if (credentials.find(cleanUsername) == credentials.end()) {
            // Perform dummy hash to prevent timing attacks
            hashPassword(cleanPassword);
            return false;
        }
        
        // Hash the provided password and compare
        std::string hashedPassword = hashPassword(cleanPassword);
        std::string storedHash = credentials[cleanUsername];
        
        // Use constant-time comparison
        return constantTimeEquals(hashedPassword, storedHash);
    }
};

// Initialize static member
std::unordered_map<std::string, std::string> Lse135::credentials;

int main() {
    // Initialize credentials
    Lse135::initializeCredentials();
    
    std::cout << "=== Authentication Test Cases ===\\n\\n";
    
    // Test case 1: Valid credentials
    std::cout << "Test 1 - Valid admin credentials:\\n";
    std::cout << "Result: " << (Lse135::check_mod("admin", "Admin@123") ? "true" : "false") << "\\n\\n";
    
    // Test case 2: Valid user credentials
    std::cout << "Test 2 - Valid user1 credentials:\\n";
    std::cout << "Result: " << (Lse135::check_mod("user1", "User@456") ? "true" : "false") << "\\n\\n";
    
    // Test case 3: Invalid password
    std::cout << "Test 3 - Invalid password:\\n";
    std::cout << "Result: " << (Lse135::check_mod("admin", "wrongpassword") ? "true" : "false") << "\\n\\n";
    
    // Test case 4: Non-existent user
    std::cout << "Test 4 - Non-existent user:\\n";
    std::cout << "Result: " << (Lse135::check_mod("hacker", "password123") ? "true" : "false") << "\\n\\n";
    
    // Test case 5: Empty credentials
    std::cout << "Test 5 - Empty credentials:\\n";
    std::cout << "Result: " << (Lse135::check_mod("", "") ? "true" : "false") << "\\n";
    
    return 0;
}
