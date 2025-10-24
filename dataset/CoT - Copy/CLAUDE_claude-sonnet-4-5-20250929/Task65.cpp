
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

class Task65 {
private:
    static const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
public:
    static std::string readFileSecurely(const std::string& filename) {
        if (filename.empty()) {
            return "Error: Filename cannot be empty";
        }
        
        try {
            // Sanitize filename - prevent path traversal
            fs::path filePath(filename);
            std::string sanitizedFilename = filePath.filename().string();
            
            // Validate filename format
            std::regex safePattern("^[a-zA-Z0-9_\\\\-\\\\.]+$");
            if (!std::regex_match(sanitizedFilename, safePattern)) {
                return "Error: Invalid filename format. Only alphanumeric, dots, hyphens, and underscores allowed";
            }
            
            fs::path fullPath = fs::canonical(fs::path(sanitizedFilename));
            
            // Check if file exists
            if (!fs::exists(fullPath)) {
                return "Error: File does not exist";
            }
            
            // Check if it's a regular file
            if (!fs::is_regular_file(fullPath)) {
                return "Error: Not a regular file";
            }
            
            // Check file size
            size_t fileSize = fs::file_size(fullPath);
            if (fileSize > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Read file content
            std::ifstream file(fullPath, std::ios::binary);
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
};

int main() {
    // Test case 1: Valid file
    std::cout << "Test 1 - Valid file:" << std::endl;
    std::cout << Task65::readFileSecurely("test.txt") << std::endl << std::endl;
    
    // Test case 2: Empty filename
    std::cout << "Test 2 - Empty filename:" << std::endl;
    std::cout << Task65::readFileSecurely("") << std::endl << std::endl;
    
    // Test case 3: Path traversal attempt
    std::cout << "Test 3 - Path traversal attempt:" << std::endl;
    std::cout << Task65::readFileSecurely("../etc/passwd") << std::endl << std::endl;
    
    // Test case 4: Invalid characters
    std::cout << "Test 4 - Invalid characters:" << std::endl;
    std::cout << Task65::readFileSecurely("file@#$.txt") << std::endl << std::endl;
    
    // Test case 5: Non-existent file
    std::cout << "Test 5 - Non-existent file:" << std::endl;
    std::cout << Task65::readFileSecurely("nonexistent.txt") << std::endl;
    
    return 0;
}
