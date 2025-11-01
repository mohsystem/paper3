
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class Lse116 {
public:
    // Secure method: Uses std::filesystem instead of system()
    static void listDirectory(const std::string& directoryPath) {
        try {
            fs::path path(directoryPath);
            
            if (!fs::exists(path)) {
                std::cout << "Directory does not exist: " << directoryPath << std::endl;
                return;
            }
            
            if (!fs::is_directory(path)) {
                std::cout << "Path is not a directory: " << directoryPath << std::endl;
                return;
            }
            
            std::cout << "Contents of " << directoryPath << ":" << std::endl;
            
            std::vector<std::string> entries;
            for (const auto& entry : fs::directory_iterator(path)) {
                entries.push_back(entry.path().filename().string());
            }
            
            std::sort(entries.begin(), entries.end());
            for (const auto& entry : entries) {
                std::cout << entry << std::endl;
            }
            
        } catch (const fs::filesystem_error& e) {
            std::cout << "Error listing directory: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "=== Test Case 1: Current directory ===" << std::endl;
    Lse116::listDirectory(".");
    
    std::cout << "\\n=== Test Case 2: Home directory ===" << std::endl;
    const char* home = getenv("HOME");
    if (home) Lse116::listDirectory(home);
    
    std::cout << "\\n=== Test Case 3: Temp directory ===" << std::endl;
    Lse116::listDirectory("/tmp");
    
    std::cout << "\\n=== Test Case 4: Non-existent directory ===" << std::endl;
    Lse116::listDirectory("/nonexistent/path");
    
    std::cout << "\\n=== Test Case 5: Invalid path ===" << std::endl;
    Lse116::listDirectory("");
    
    return 0;
}
