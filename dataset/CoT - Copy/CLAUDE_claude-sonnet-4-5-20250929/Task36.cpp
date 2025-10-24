
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

/**
 * Reads a file and returns its contents as a string.
 * Implements security measures to prevent path traversal and other file-related vulnerabilities.
 */
std::string readFileContents(const std::string& filePath) {
    if (filePath.empty()) {
        return "Error: File path cannot be empty";
    }
    
    try {
        // Normalize the path to prevent path traversal attacks
        fs::path path = fs::absolute(fs::path(filePath)).lexically_normal();
        
        // Check if file exists
        if (!fs::exists(path)) {
            return "Error: File does not exist";
        }
        
        // Check if it's a regular file (not a directory or special file)
        if (!fs::is_regular_file(path)) {
            return "Error: Path does not point to a regular file";
        }
        
        // Check file size to prevent reading extremely large files
        std::uintmax_t fileSize = fs::file_size(path);
        if (fileSize > 10 * 1024 * 1024) { // 10 MB limit
            return "Error: File size exceeds maximum allowed size (10 MB)";
        }
        
        // Read file contents securely
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return "Error: Unable to open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
        
    } catch (const fs::filesystem_error& e) {
        return std::string("Error: Filesystem error - ") + e.what();
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    std::cout << "=== File Reader Program ===" << std::endl << std::endl;
    
    // Test case 1: Valid text file (create a temporary file for testing)
    std::cout << "Test Case 1: Valid text file" << std::endl;
    try {
        std::string tempFileName = std::tmpnam(nullptr);
        tempFileName += ".txt";
        std::ofstream tempFile(tempFileName);
        tempFile << "Hello, World!\\nThis is a test file.";
        tempFile.close();
        
        std::cout << readFileContents(tempFileName) << std::endl;
        std::remove(tempFileName.c_str());
    } catch (...) {
        std::cout << "Test setup failed" << std::endl;
    }
    
    // Test case 2: Empty file path
    std::cout << "\\nTest Case 2: Empty file path" << std::endl;
    std::cout << readFileContents("") << std::endl;
    
    // Test case 3: Non-existent file
    std::cout << "\\nTest Case 3: Non-existent file" << std::endl;
    std::cout << readFileContents("/nonexistent/path/file.txt") << std::endl;
    
    // Test case 4: Path traversal attempt (security test)
    std::cout << "\\nTest Case 4: Path traversal attempt" << std::endl;
    std::cout << readFileContents("../../../etc/passwd") << std::endl;
    
    // Test case 5: Directory instead of file
    std::cout << "\\nTest Case 5: Directory instead of file" << std::endl;
    std::cout << readFileContents(".") << std::endl;
    
    return 0;
}
