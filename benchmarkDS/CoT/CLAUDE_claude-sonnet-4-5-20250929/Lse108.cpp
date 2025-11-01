
#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <memory>
#include <cstdio>

class Lse108 {
private:
    static bool isValidUsername(const std::string& username) {
        // Validate username: alphanumeric, underscore, hyphen only, max 32 chars
        std::regex valid_pattern("^[a-zA-Z0-9_-]{1,32}$");
        return std::regex_match(username, valid_pattern);
    }
    
public:
    static std::string getPasswdInfo(const std::string& username) {
        // Input validation to prevent command injection
        if (username.empty() || !isValidUsername(username)) {
            return "Error: Invalid username format";
        }
        
        // Safely construct command with validation
        std::string command = "getent passwd " + username;
        std::string result;
        
        // Use popen for safer execution (still needs validation)
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        
        std::array<char, 128> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        
        int status = pclose(pipe);
        if (status != 0 && result.empty()) {
            return "Error: User not found or command failed";
        }
        
        return result;
    }
};

int main() {
    // Test cases
    std::cout << "Test 1 - Valid user 'root':" << std::endl;
    std::cout << Lse108::getPasswdInfo("root") << std::endl;
    
    std::cout << "\\nTest 2 - Valid user 'nobody':" << std::endl;
    std::cout << Lse108::getPasswdInfo("nobody") << std::endl;
    
    std::cout << "\\nTest 3 - Invalid user with special chars '; rm -rf /':" << std::endl;
    std::cout << Lse108::getPasswdInfo("; rm -rf /") << std::endl;
    
    std::cout << "\\nTest 4 - Empty username:" << std::endl;
    std::cout << Lse108::getPasswdInfo("") << std::endl;
    
    std::cout << "\\nTest 5 - Username with spaces 'root user':" << std::endl;
    std::cout << Lse108::getPasswdInfo("root user") << std::endl;
    
    return 0;
}
