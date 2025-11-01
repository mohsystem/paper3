
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <string>

int executeIdCommand(const std::string& username) {
    if (username.empty()) {
        return -1;
    }
    
    // Sanitize username to prevent command injection
    std::regex validUsernamePattern("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(username, validUsernamePattern)) {
        std::cerr << "Invalid username format" << std::endl;
        return -1;
    }
    
    std::string command = "id " + username;
    int exitStatus = system(command.c_str());
    
    // Extract actual exit status from system() return value
    if (exitStatus == -1) {
        return -1;
    }
    
    return WEXITSTATUS(exitStatus);
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {"root", "nobody", "daemon", "invaliduser123", "admin"};
    
    if (argc > 1) {
        // Use command line argument if provided
        int exitStatus = executeIdCommand(argv[1]);
        std::cout << "Exit status: " << exitStatus << std::endl;
    } else {
        // Run test cases
        std::cout << "Running test cases:\\n" << std::endl;
        for (const auto& username : testCases) {
            std::cout << "Testing username: " << username << std::endl;
            int exitStatus = executeIdCommand(username);
            std::cout << "Exit status: " << exitStatus << std::endl;
            std::cout << "---" << std::endl;
        }
    }
    
    return 0;
}
