
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Lse137 {
private:
    static std::map<std::string, std::string> moderators;
    static std::map<std::string, std::string> validSessions;
    
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
    
    static std::string sanitizeInput(const std::string& input) {
        std::regex pattern("[^a-zA-Z0-9]");
        return std::regex_replace(input, pattern, "");
    }
    
public:
    static void initialize() {
        moderators["admin"] = hashPassword("admin123");
        moderators["mod1"] = hashPassword("modpass456");
        validSessions["session123"] = "admin";
        validSessions["session456"] = "mod1";
    }
    
    static bool auth_mod(std::string username, std::string password, std::string session) {
        try {
            // Sanitize inputs
            username = sanitizeInput(username);
            session = sanitizeInput(session);
            
            if (username.empty() || password.empty() || session.empty()) {
                return false;
            }
            
            // Verify session
            auto sessionIt = validSessions.find(session);
            if (sessionIt == validSessions.end() || sessionIt->second != username) {
                return false;
            }
            
            // Verify credentials
            auto modIt = moderators.find(username);
            if (modIt == moderators.end()) {
                return false;
            }
            
            std::string inputHash = hashPassword(password);
            return modIt->second == inputHash;
            
        } catch (...) {
            return false;
        }
    }
    
    static std::string check_mod(const std::string& username, 
                                  const std::string& password, 
                                  const std::string& session) {
        bool isModerator = auth_mod(username, password, session);
        return isModerator ? "Moderator access granted" : "Access denied";
    }
};

std::map<std::string, std::string> Lse137::moderators;
std::map<std::string, std::string> Lse137::validSessions;

int main() {
    Lse137::initialize();
    
    // Test cases
    std::cout << "Test Case 1: Valid moderator admin" << std::endl;
    std::cout << Lse137::check_mod("admin", "admin123", "session123") << std::endl;
    
    std::cout << "\\nTest Case 2: Valid moderator mod1" << std::endl;
    std::cout << Lse137::check_mod("mod1", "modpass456", "session456") << std::endl;
    
    std::cout << "\\nTest Case 3: Invalid password" << std::endl;
    std::cout << Lse137::check_mod("admin", "wrongpass", "session123") << std::endl;
    
    std::cout << "\\nTest Case 4: Invalid session" << std::endl;
    std::cout << Lse137::check_mod("admin", "admin123", "invalidsession") << std::endl;
    
    std::cout << "\\nTest Case 5: Non-existent user" << std::endl;
    std::cout << Lse137::check_mod("hacker", "password", "session999") << std::endl;
    
    return 0;
}
