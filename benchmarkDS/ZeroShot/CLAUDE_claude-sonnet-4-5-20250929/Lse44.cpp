
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

const std::string SAFE_DIRECTORY = "/safe/";

std::string readFileFromSafeDirectory(const std::string& requestedFile) {
    try {
        // Normalize and resolve the path
        fs::path safePath = fs::absolute(fs::path(SAFE_DIRECTORY)).lexically_normal();
        fs::path requestedPath = fs::absolute(safePath / requestedFile).lexically_normal();
        
        // Security check: ensure the resolved path is still within the safe directory
        auto safePathStr = safePath.string();
        auto requestedPathStr = requestedPath.string();
        
        if (requestedPathStr.find(safePathStr) != 0) {
            return "Error: Access denied - Path traversal attempt detected";
        }
        
        // Check if file exists
        if (!fs::exists(requestedPath)) {
            return "Error: File not found";
        }
        
        // Read and return file content
        std::ifstream file(requestedPath);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception& e) {
        return std::string("Error: Unable to read file - ") + e.what();
    }
}

int main() {
    // Test cases
    std::cout << "Test 1 - Normal file:" << std::endl;
    std::cout << readFileFromSafeDirectory("test.txt") << std::endl;
    
    std::cout << "\\nTest 2 - Path traversal attempt:" << std::endl;
    std::cout << readFileFromSafeDirectory("../../../etc/passwd") << std::endl;
    
    std::cout << "\\nTest 3 - Another path traversal:" << std::endl;
    std::cout << readFileFromSafeDirectory("..\\\\..\\\\windows\\\\system32\\\\config\\\\sam") << std::endl;
    
    std::cout << "\\nTest 4 - Non-existent file:" << std::endl;
    std::cout << readFileFromSafeDirectory("nonexistent.txt") << std::endl;
    
    std::cout << "\\nTest 5 - Subdirectory file:" << std::endl;
    std::cout << readFileFromSafeDirectory("subdir/file.txt") << std::endl;
    
    return 0;
}
