
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <array>

class Lse110 {
private:
    // Username validation pattern: alphanumeric, dash, underscore only
    static bool isValidUsername(const std::string& username) {
        std::regex pattern("^[a-zA-Z0-9_-]+$");
        return std::regex_match(username, pattern);
    }
    
public:
    static int getUserInfo(const std::string& username) {
        // Input validation to prevent command injection
        if (username.empty()) {
            std::cerr << "Error: Username cannot be empty" << std::endl;
            return -1;
        }
        
        if (!isValidUsername(username)) {
            std::cerr << "Error: Invalid username format" << std::endl;
            return -1;
        }
        
        // Build secure command string
        std::string command = "id " + username + " 2>&1";
        
        // Execute command securely
        int exitStatus = system(command.c_str()) >> 8;
        return exitStatus;
    }
};

int main(int argc, char* argv[]) {
    // Test cases
    std::array<std::string, 5> testUsers = {
        "root", "nobody", "invaliduser123", "test-user", "invalid;user"
    };
    
    if (argc > 1) {
        // Use command line argument
        int exitStatus = Lse110::getUserInfo(argv[1]);
        std::cout << "Exit status: " << exitStatus << std::endl;
    } else {
        // Run test cases
        std::cout << "Running test cases:" << std::endl;
        for (const auto& user : testUsers) {
            std::cout << "\\nTesting user: " << user << std::endl;
            int exitStatus = Lse110::getUserInfo(user);
            std::cout << "Exit status: " << exitStatus << std::endl;
        }
    }
    
    return 0;
}
