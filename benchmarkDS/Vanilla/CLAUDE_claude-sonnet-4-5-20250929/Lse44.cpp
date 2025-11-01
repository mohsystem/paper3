
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

std::string readFileFromSafeDirectory(const std::string& filename) {
    try {
        // Construct the safe directory path
        std::string safeDirectory = "/safe/";
        std::string filePath = safeDirectory + filename;
        
        // Basic security check for path traversal
        if (filename.find("..") != std::string::npos) {
            return "Error: Access denied - path traversal attempt detected";
        }
        
        // Read the file content
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "Error reading file: Could not open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
    } catch (const std::exception& e) {
        return std::string("Error reading file: ") + e.what();
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1: Reading 'document.txt'" << std::endl;
    std::cout << readFileFromSafeDirectory("document.txt") << std::endl;
    
    std::cout << "\\nTest Case 2: Reading 'data.txt'" << std::endl;
    std::cout << readFileFromSafeDirectory("data.txt") << std::endl;
    
    std::cout << "\\nTest Case 3: Path traversal attempt '../etc/passwd'" << std::endl;
    std::cout << readFileFromSafeDirectory("../etc/passwd") << std::endl;
    
    std::cout << "\\nTest Case 4: Reading subdirectory file 'subdir/file.txt'" << std::endl;
    std::cout << readFileFromSafeDirectory("subdir/file.txt") << std::endl;
    
    std::cout << "\\nTest Case 5: Non-existent file 'nonexistent.txt'" << std::endl;
    std::cout << readFileFromSafeDirectory("nonexistent.txt") << std::endl;
    
    return 0;
}
