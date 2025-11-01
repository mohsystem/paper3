
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

const std::string SAFE_DIRECTORY = "/safe/";

std::string readSafeFile(const std::string& filename) {
    if (filename.empty()) {
        return "Error: Filename cannot be empty";
    }
    
    try {
        // Get canonical path of safe directory
        std::filesystem::path safePath = std::filesystem::canonical(SAFE_DIRECTORY);
        
        // Construct requested path
        std::filesystem::path requestedPath = safePath / filename;
        
        // Normalize and get canonical path if exists
        if (std::filesystem::exists(requestedPath)) {
            requestedPath = std::filesystem::canonical(requestedPath);
        } else {
            requestedPath = std::filesystem::weakly_canonical(requestedPath);
        }
        
        // Verify the resolved path is within safe directory
        auto relativePath = std::filesystem::relative(requestedPath, safePath);
        if (relativePath.empty() || relativePath.string().substr(0, 2) == "..") {
            return "Error: Access denied - file outside safe directory";
        }
        
        // Check if file exists and is regular file
        if (!std::filesystem::exists(requestedPath) || 
            !std::filesystem::is_regular_file(requestedPath)) {
            return "Error: File not found or not readable";
        }
        
        // Read file content
        std::ifstream file(requestedPath);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception& e) {
        return "Error: Unable to read file";
    }
}

int main() {
    // Test cases
    std::cout << "Test 1 - Normal file:" << std::endl;
    std::cout << readSafeFile("test.txt") << std::endl;
    
    std::cout << "\\nTest 2 - Path traversal attempt:" << std::endl;
    std::cout << readSafeFile("../etc/passwd") << std::endl;
    
    std::cout << "\\nTest 3 - Empty filename:" << std::endl;
    std::cout << readSafeFile("") << std::endl;
    
    std::cout << "\\nTest 4 - Path with dots:" << std::endl;
    std::cout << readSafeFile("../../sensitive.txt") << std::endl;
    
    std::cout << "\\nTest 5 - Non-existent file:" << std::endl;
    std::cout << readSafeFile("nonexistent.txt") << std::endl;
    
    return 0;
}
