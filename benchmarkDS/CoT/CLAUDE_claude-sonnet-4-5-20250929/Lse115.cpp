
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// SECURE: Uses C++17 filesystem library instead of executing shell commands
void listDirectory(const std::string& directoryPath) {
    try {
        fs::path path(directoryPath);
        
        // Validate path exists and is a directory
        if (!fs::exists(path) || !fs::is_directory(path)) {
            std::cout << "Invalid directory: " << directoryPath << std::endl;
            return;
        }
        
        std::cout << "Contents of " << directoryPath << ":" << std::endl;
        for (const auto& entry : fs::directory_iterator(path)) {
            std::cout << entry.path().filename().string() << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }
}

int main() {
    // Test cases using safe directory listing
    std::cout << "Test 1: Current directory" << std::endl;
    listDirectory(".");
    
    std::cout << "\\nTest 2: Parent directory" << std::endl;
    listDirectory("..");
    
    std::cout << "\\nTest 3: Temp directory" << std::endl;
    listDirectory("/tmp");
    
    std::cout << "\\nTest 4: Home directory" << std::endl;
    listDirectory(getenv("HOME") ? getenv("HOME") : ".");
    
    std::cout << "\\nTest 5: Invalid directory" << std::endl;
    listDirectory("/nonexistent/path");
    
    return 0;
}
