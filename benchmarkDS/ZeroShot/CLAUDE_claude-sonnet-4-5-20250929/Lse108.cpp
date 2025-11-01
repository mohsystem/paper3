
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <regex>

class Lse108 {
public:
    static std::string executeGetentPasswd(const std::string& username) {
        // Input validation
        if (username.empty()) {
            return "Error: Username cannot be empty";
        }
        
        // Sanitize username - only allow alphanumeric, underscore, and hyphen
        std::regex validPattern("^[a-zA-Z0-9_-]+$");
        if (!std::regex_match(username, validPattern)) {
            return "Error: Invalid username format";
        }
        
        // Build safe command
        std::string command = "getent passwd " + username;
        
        std::array<char, 128> buffer;
        std::string result;
        
        // Use popen for command execution
        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen(command.c_str(), "r"), pclose
        );
        
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        
        // Read output
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // Remove trailing newline
        if (!result.empty() && result.back() == '\\n') {
            result.pop_back();
        }
        
        return result.empty() ? "Error: User not found" : result;
    }
};

int main() {
    // Test cases
    std::string testUsers[] = {"root", "nobody", "daemon", "testuser", "admin"};
    
    std::cout << "Testing getent passwd command:" << std::endl;
    std::cout << "================================\\n" << std::endl;
    
    for (const auto& username : testUsers) {
        std::cout << "User: " << username << std::endl;
        std::string result = Lse108::executeGetentPasswd(username);
        std::cout << "Result: " << result << std::endl;
        std::cout << "--------------------------------\\n" << std::endl;
    }
    
    return 0;
}
