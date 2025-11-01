
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

/**
 * Securely reads a file from the /safe/ directory
 * @param filename The name of the file to read (without path)
 * @return The content of the file as a string, or error message
 */
std::string readSafeFile(const std::string& filename) {
    try {
        // Validate filename
        if (filename.empty()) {
            return "Error: Filename cannot be empty";
        }
        
        // Extract just the filename (remove path components)
        fs::path temp(filename);
        std::string sanitizedFilename = temp.filename().string();
        
        // Construct the safe path
        fs::path safePath = fs::path("/safe/") / sanitizedFilename;
        safePath = fs::weakly_canonical(safePath);
        
        // Verify the path is still within /safe/ directory
        std::string safePathStr = safePath.string();
        if (safePathStr.find("/safe/") != 0) {
            return "Error: Access denied - Path traversal attempt detected";
        }
        
        // Check if file exists
        if (!fs::exists(safePath)) {
            return "Error: File not found";
        }
        
        // Read the file
        std::ifstream file(safePath);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1: Normal file" << std::endl;
    std::cout << readSafeFile("test.txt") << std::endl << std::endl;
    
    std::cout << "Test Case 2: Path traversal attempt with ../" << std::endl;
    std::cout << readSafeFile("../etc/passwd") << std::endl << std::endl;
    
    std::cout << "Test Case 3: Absolute path attempt" << std::endl;
    std::cout << readSafeFile("/etc/passwd") << std::endl << std::endl;
    
    std::cout << "Test Case 4: Empty filename" << std::endl;
    std::cout << readSafeFile("") << std::endl << std::endl;
    
    std::cout << "Test Case 5: Complex path traversal" << std::endl;
    std::cout << readSafeFile("../../../../../../etc/passwd") << std::endl;
    
    return 0;
}
