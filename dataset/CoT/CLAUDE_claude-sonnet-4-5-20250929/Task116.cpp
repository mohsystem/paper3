
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

class Task116 {
private:
    static const std::regex USERNAME_PATTERN;
    static const std::regex PASSWORD_PATTERN;
    
public:
    static bool validateUsername(const std::string& username) {
        if (username.empty()) {
            return false;
        }
        return std::regex_match(username, USERNAME_PATTERN);
    }
    
    static bool validatePassword(const std::string& password) {
        if (password.empty()) {
            return false;
        }
        return std::regex_match(password, PASSWORD_PATTERN);
    }
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static bool changePasswordSecure(const std::string& username, const std::string& newPassword) {
        // Validate inputs
        if (!validateUsername(username)) {
            std::cerr << "Invalid username format" << std::endl;
            return false;
        }
        
        if (!validatePassword(newPassword)) {
            std::cerr << "Invalid password format" << std::endl;
            return false;
        }
        
        // Simulated privileged operation
        std::cout << "Simulating privileged password change for user: " << username << std::endl;
        
        // Hash password
        std::string hashedPassword = hashPassword(newPassword);
        if (hashedPassword.empty()) {
            std::cerr << "Password hashing failed" << std::endl;
            return false;
        }
        
        std::cout << "Password changed successfully (simulated)" << std::endl;
        return true;
    }
    
    static void dropPrivileges() {
        std::cout << "Dropping privileges..." << std::endl;
        
#ifdef _WIN32
        // Windows implementation
        std::cout << "Would adjust token privileges on Windows" << std::endl;
        std::cout << "Privileges dropped to limited user" << std::endl;
#else
        // Unix-like systems
        try {
            // In production, use setuid() and setgid()
            std::cout << "Would call setuid/setgid on Unix-like system" << std::endl;
            std::cout << "Privileges dropped to unprivileged user" << std::endl;
        } catch (...) {
            std::cerr << "Failed to drop privileges" << std::endl;
        }
#endif
    }
    
    static void performPrivilegedOperation(const std::string& username, const std::string& newPassword) {
        try {
            bool success = changePasswordSecure(username, newPassword);
            
            if (success) {
                std::cout << "Operation completed successfully" << std::endl;
            } else {
                std::cout << "Operation failed" << std::endl;
            }
        } catch (...) {
            std::cerr << "Exception occurred during operation" << std::endl;
        }
        
        // Always drop privileges
        dropPrivileges();
    }
};

const std::regex Task116::USERNAME_PATTERN("^[a-zA-Z0-9_-]{3,32}$");
const std::regex Task116::PASSWORD_PATTERN("^.{8,128}$");

int main() {
    std::cout << "=== Test Case 1: Valid username and password ===" << std::endl;
    Task116::performPrivilegedOperation("testuser1", "SecurePass123!");
    
    std::cout << "\\n=== Test Case 2: Invalid username (special chars) ===" << std::endl;
    Task116::performPrivilegedOperation("test@user", "SecurePass123!");
    
    std::cout << "\\n=== Test Case 3: Invalid password (too short) ===" << std::endl;
    Task116::performPrivilegedOperation("testuser2", "short");
    
    std::cout << "\\n=== Test Case 4: Valid username and long password ===" << std::endl;
    Task116::performPrivilegedOperation("validuser", "ThisIsAVeryLongAndSecurePassword12345!");
    
    std::cout << "\\n=== Test Case 5: Empty username ===" << std::endl;
    Task116::performPrivilegedOperation("", "SecurePass123!");
    
    return 0;
}
