
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

const std::string SAFE_DIRECTORY = "/var/safe_files/";

std::string readFileFromSafeDirectory(const std::string& filename) {
    try {
        // Input validation
        if (filename.empty()) {
            return "Error: Invalid filename";
        }
        
        // Sanitize filename - remove any path separators
        std::string sanitizedFilename = filename;
        sanitizedFilename.erase(
            std::remove(sanitizedFilename.begin(), sanitizedFilename.end(), '/'),
            sanitizedFilename.end()
        );
        sanitizedFilename.erase(
            std::remove(sanitizedFilename.begin(), sanitizedFilename.end(), '\\\\'),\n            sanitizedFilename.end()\n        );\n        \n        // Create paths and resolve to canonical paths\n        fs::path safeDir = fs::canonical(SAFE_DIRECTORY);\n        fs::path requestedFile = safeDir / sanitizedFilename;\n        requestedFile = fs::canonical(requestedFile);\n        \n        // Security check: ensure the resolved path is within safe directory\n        std::string safeDirStr = safeDir.string();\n        std::string requestedFileStr = requestedFile.string();\n        \n        if (requestedFileStr.find(safeDirStr) != 0) {\n            return "Error: Access denied - file outside safe directory";\n        }\n        \n        // Check if file exists\n        if (!fs::exists(requestedFile)) {\n            return "Error: File not found";\n        }\n        \n        // Check if it's a regular file
        if (!fs::is_regular_file(requestedFile)) {
            return "Error: Not a regular file";
        }
        
        // Read and return file content
        std::ifstream file(requestedFile);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const fs::filesystem_error&) {
        return "Error: Filesystem error";
    } catch (const std::exception&) {
        return "Error: An error occurred";
    }
}

int main() {
    // Test cases
    std::cout << "Test 1 - Valid file:" << std::endl;
    std::cout << readFileFromSafeDirectory("document.txt") << std::endl;
    
    std::cout << "\\nTest 2 - Path traversal attempt:" << std::endl;
    std::cout << readFileFromSafeDirectory("../etc/passwd") << std::endl;
    
    std::cout << "\\nTest 3 - Absolute path attempt:" << std::endl;
    std::cout << readFileFromSafeDirectory("/etc/passwd") << std::endl;
    
    std::cout << "\\nTest 4 - Empty filename:" << std::endl;
    std::cout << readFileFromSafeDirectory("") << std::endl;
    
    std::cout << "\\nTest 5 - Non-existent file:" << std::endl;
    std::cout << readFileFromSafeDirectory("nonexistent.txt") << std::endl;
    
    return 0;
}
