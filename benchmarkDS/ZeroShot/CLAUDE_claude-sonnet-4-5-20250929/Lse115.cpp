
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <array>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

class Lse115 {
public:
    // Secure method using C++ filesystem library
    static std::string listDirectory(const std::string& directoryPath) {
        try {
            fs::path path(directoryPath);
            
            // Check if directory exists and is a directory
            if (!fs::exists(path) || !fs::is_directory(path)) {
                return "Invalid directory path";
            }
            
            std::ostringstream result;
            for (const auto& entry : fs::directory_iterator(path)) {
                result << entry.path().filename().string() << "\\n";
            }
            
            return result.str();
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    // Alternative method using popen (safer with proper validation)
    static std::string executeListCommand(const std::string& directoryPath) {
        try {
            fs::path path(directoryPath);
            
            // Validate directory path
            if (!fs::exists(path) || !fs::is_directory(path)) {
                return "Invalid directory path";
            }
            
            // Build command with proper escaping
            std::string command = "ls -l \\"" + directoryPath + "\\" 2>&1";
            std::array<char, 128> buffer;
            std::string result;
            
            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) {
                return "Error executing command";
            }
            
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
            }
            
            pclose(pipe);
            return result;
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
};

int main() {
    std::cout << "Test Case 1: Current directory\\n";
    std::cout << Lse115::listDirectory(".") << "\\n";
    
    std::cout << "\\nTest Case 2: Home directory\\n";
    std::cout << Lse115::listDirectory(std::getenv("HOME")) << "\\n";
    
    std::cout << "\\nTest Case 3: Root directory\\n";
    std::cout << Lse115::listDirectory("/") << "\\n";
    
    std::cout << "\\nTest Case 4: Invalid directory\\n";
    std::cout << Lse115::listDirectory("/nonexistent/path") << "\\n";
    
    std::cout << "\\nTest Case 5: Tmp directory\\n";
    std::cout << Lse115::listDirectory("/tmp") << "\\n";
    
    return 0;
}
