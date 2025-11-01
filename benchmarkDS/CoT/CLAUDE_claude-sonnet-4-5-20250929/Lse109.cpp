
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

class Lse109 {
private:
    static bool isValidUsername(const std::string& username) {
        std::regex pattern("^[a-zA-Z0-9_-]{1,32}$");
        return std::regex_match(username, pattern);
    }

public:
    static std::string getUserInfo(const std::string& username) {
        // Validate username to prevent injection attacks
        if (username.empty() || !isValidUsername(username)) {
            return "Invalid username format";
        }
        
        std::ifstream file("/etc/passwd");
        if (!file.is_open()) {
            return "File not found: /etc/passwd";
        }
        
        std::string result;
        std::string line;
        std::string searchStr = username + ":";
        
        while (std::getline(file, line)) {
            // Safely check if line starts with username followed by colon
            if (line.compare(0, searchStr.length(), searchStr) == 0) {
                result += line + "\\n";
            }
        }
        
        file.close();
        
        if (!result.empty()) {
            result.pop_back(); // Remove trailing newline
            return result;
        }
        
        return "User not found";
    }
};

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {"root", "daemon", "nobody", "invalid@user", "testuser"};
    
    if (argc > 1) {
        std::cout << Lse109::getUserInfo(argv[1]) << std::endl;
    } else {
        std::cout << "Running test cases:" << std::endl;
        for (const auto& username : testCases) {
            std::cout << "\\n[Test: " << username << "]" << std::endl;
            std::cout << Lse109::getUserInfo(username) << std::endl;
        }
    }
    
    return 0;
}
