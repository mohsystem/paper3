
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <sys/stat.h>

namespace fs = std::filesystem;

std::string readFileFromSafeDirectory(const std::string& filename, const std::string& safeDirectory) {
    try {
        // Normalize and resolve paths
        fs::path safePath = fs::canonical(safeDirectory);
        fs::path requestedPath = fs::weakly_canonical(safePath / filename);
        
        // Check if the requested path is within the safe directory
        auto safeStr = safePath.string();
        auto requestedStr = requestedPath.string();
        
        if (requestedStr.find(safeStr) != 0) {
            return "Error: Access denied - file is outside safe directory";
        }
        
        // Check if file exists
        if (!fs::exists(requestedPath)) {
            return "Error: File not found";
        }
        
        // Read file content
        std::ifstream file(requestedPath);
        if (!file.is_open()) {
            return "Error: Cannot open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Create a test safe directory and files
    std::string safeDir = "safe_directory";
    fs::create_directories(safeDir);
    
    std::ofstream file1(safeDir + "/test1.txt");
    file1 << "Content of test1";
    file1.close();
    
    std::ofstream file2(safeDir + "/test2.txt");
    file2 << "Content of test2";
    file2.close();
    
    // Test case 1: Valid file in safe directory
    std::cout << "Test 1: " << readFileFromSafeDirectory("test1.txt", safeDir) << std::endl;
    
    // Test case 2: Another valid file
    std::cout << "Test 2: " << readFileFromSafeDirectory("test2.txt", safeDir) << std::endl;
    
    // Test case 3: Non-existent file
    std::cout << "Test 3: " << readFileFromSafeDirectory("nonexistent.txt", safeDir) << std::endl;
    
    // Test case 4: Path traversal attempt
    std::cout << "Test 4: " << readFileFromSafeDirectory("../test.txt", safeDir) << std::endl;
    
    // Test case 5: Another path traversal attempt
    std::cout << "Test 5: " << readFileFromSafeDirectory("../../etc/passwd", safeDir) << std::endl;
    
    return 0;
}
