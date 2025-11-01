
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string readFileFromSafeDirectory(const std::string& filename) {
    try {
        // Define the safe directory
        fs::path safeDir = fs::absolute("/safe/").lexically_normal();
        
        // Construct the file path
        fs::path filePath = fs::absolute(safeDir / filename).lexically_normal();
        
        // Security check: ensure the resolved path is within safe directory
        auto safeDirStr = safeDir.string();
        auto filePathStr = filePath.string();
        
        if (filePathStr.find(safeDirStr) != 0) {
            return "Error: Access denied - path traversal detected";
        }
        
        // Check if file exists and is a regular file
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            return "Error: File does not exist or is not a regular file";
        }
        
        // Read the file content
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
        
    } catch (const fs::filesystem_error& e) {
        return "Error: Filesystem error occurred";
    } catch (const std::exception& e) {
        return "Error: An unexpected error occurred";
    }
}

int main() {
    // Test case 1: Normal file
    std::cout << "Test 1 - Normal file:" << std::endl;
    std::cout << readFileFromSafeDirectory("test.txt") << std::endl;
    
    // Test case 2: Path traversal attempt
    std::cout << "\\nTest 2 - Path traversal attempt:" << std::endl;
    std::cout << readFileFromSafeDirectory("../etc/passwd") << std::endl;
    
    // Test case 3: Another path traversal
    std::cout << "\\nTest 3 - Another traversal:" << std::endl;
    std::cout << readFileFromSafeDirectory("../../secret.txt") << std::endl;
    
    // Test case 4: Subdirectory file
    std::cout << "\\nTest 4 - Subdirectory:" << std::endl;
    std::cout << readFileFromSafeDirectory("subdir/file.txt") << std::endl;
    
    // Test case 5: Non-existent file
    std::cout << "\\nTest 5 - Non-existent file:" << std::endl;
    std::cout << readFileFromSafeDirectory("nonexistent.txt") << std::endl;
    
    return 0;
}
