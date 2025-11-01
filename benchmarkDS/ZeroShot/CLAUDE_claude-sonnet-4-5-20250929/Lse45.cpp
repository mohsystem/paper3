
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

const std::string SAFE_DIRECTORY = "/var/safe_files/";

std::string readFileFromSafeDirectory(const std::string& filename) {
    try {
        // Sanitize the filename to prevent path traversal attacks
        fs::path requestedPath(filename);
        std::string sanitizedFilename = requestedPath.filename().string();
        
        // Construct the full path within the safe directory
        fs::path safePath = fs::canonical(fs::absolute(SAFE_DIRECTORY));
        fs::path filePath = safePath / sanitizedFilename;
        
        // Normalize the file path
        if (fs::exists(filePath)) {
            filePath = fs::canonical(filePath);
        }
        
        // Verify the file is still within the safe directory
        std::string filePathStr = filePath.string();
        std::string safePathStr = safePath.string();
        if (filePathStr.find(safePathStr) != 0) {
            return "Error: Path traversal attempt detected";
        }
        
        // Check if file exists
        if (!fs::exists(filePath)) {
            return "Error: File not found";
        }
        
        // Read and return file content
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "Error: Cannot open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

int main() {
    // Test cases
    std::cout << "Test 1: " << readFileFromSafeDirectory("document.txt") << std::endl;
    std::cout << "Test 2: " << readFileFromSafeDirectory("data.json") << std::endl;
    std::cout << "Test 3: " << readFileFromSafeDirectory("../etc/passwd") << std::endl;
    std::cout << "Test 4: " << readFileFromSafeDirectory("nonexistent.txt") << std::endl;
    std::cout << "Test 5: " << readFileFromSafeDirectory("config.ini") << std::endl;
    
    return 0;
}
