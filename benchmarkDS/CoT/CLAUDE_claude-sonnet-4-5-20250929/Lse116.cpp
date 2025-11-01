
// C++ - Secure alternative using popen with validation
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdio>
#include <algorithm>

class Lse116 {
public:
    // SECURE: Validates input and uses controlled command execution
    static std::string executeListCommand(const std::string& directory) {
        // Validate directory path to prevent injection
        if (directory.empty() || 
            directory.find("..") != std::string::npos ||
            directory.find(";") != std::string::npos ||
            directory.find("|") != std::string::npos ||
            directory.find("&") != std::string::npos ||
            directory.find("`") != std::string::npos) {
            return "Error: Invalid directory path\\n";
        }
        
        // Build safe command - only allow alphanumeric, /, ., -, _
        std::string safeDir = directory;
        for (char c : safeDir) {
            if (!isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_' && c != ' ') {
                return "Error: Invalid characters in path\\n";
            }
        }
        
        std::string command = "ls -l \\"" + safeDir + "\\" 2>&1";
        std::array<char, 128> buffer;
        std::string result;
        
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            return "Error: Failed to execute command\\n";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }
};

int main() {
    std::cout << "Test Case 1: Current directory\\n";
    std::cout << Lse116::executeListCommand(".") << "\\n";
    
    std::cout << "Test Case 2: Root directory\\n";
    std::cout << Lse116::executeListCommand("/") << "\\n";
    
    std::cout << "Test Case 3: Temp directory\\n";
    std::cout << Lse116::executeListCommand("/tmp") << "\\n";
    
    std::cout << "Test Case 4: Invalid path with traversal\\n";
    std::cout << Lse116::executeListCommand("../../../etc") << "\\n";
    
    std::cout << "Test Case 5: Command injection attempt\\n";
    std::cout << Lse116::executeListCommand("; rm -rf /") << "\\n";
    
    return 0;
}
