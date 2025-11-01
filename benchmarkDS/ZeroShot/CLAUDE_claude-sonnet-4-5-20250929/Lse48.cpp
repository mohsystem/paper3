
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

const std::string ALLOWED_DIRECTORY = "./images/";

std::vector<char> img(const std::string& fileParam) {
    std::vector<char> result;
    
    if (fileParam.empty()) {
        std::cerr << "File parameter is required" << std::endl;
        return result;
    }
    
    // Secure: Sanitize input to prevent path traversal
    std::string sanitizedFile = std::regex_replace(fileParam, std::regex("[^a-zA-Z0-9._-]"), "");
    
    // Prevent path traversal attempts
    if (sanitizedFile.find("..") != std::string::npos || 
        sanitizedFile.find("/") != std::string::npos || 
        sanitizedFile.find("\\\\") != std::string::npos) {\n        std::cerr << "Invalid file name" << std::endl;\n        return result;\n    }\n    \n    try {\n        fs::path allowedPath = fs::canonical(fs::path(ALLOWED_DIRECTORY));\n        fs::path filePath = fs::canonical(fs::path(ALLOWED_DIRECTORY) / sanitizedFile);\n        \n        // Ensure the file is within allowed directory\n        std::string filePathStr = filePath.string();\n        std::string allowedPathStr = allowedPath.string();\n        \n        if (filePathStr.find(allowedPathStr) != 0) {\n            std::cerr << "Access denied: Path traversal attempt detected" << std::endl;\n            return result;\n        }\n        \n        // Check if file exists and is regular file\n        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {\n            std::cerr << "File not found or not readable" << std::endl;\n            return result;\n        }\n        \n        // Read file in binary mode\n        std::ifstream file(filePath, std::ios::binary | std::ios::ate);\n        if (!file.is_open()) {\n            std::cerr << "Error opening file" << std::endl;\n            return result;\n        }\n        \n        std::streamsize size = file.tellg();\n        file.seekg(0, std::ios::beg);\n        \n        result.resize(size);\n        if (!file.read(result.data(), size)) {\n            std::cerr << "Error reading file" << std::endl;\n            result.clear();\n        }\n        \n        file.close();\n    } catch (const std::exception& e) {\n        std::cerr << "Error: " << e.what() << std::endl;\n        result.clear();\n    }\n    \n    return result;\n}\n\nint main() {\n    // Create test directory and files\n    try {\n        fs::create_directories(ALLOWED_DIRECTORY);\n        \n        std::ofstream file1(ALLOWED_DIRECTORY + "test1.txt", std::ios::binary);\n        file1 << "Test content 1";\n        file1.close();\n        \n        std::ofstream file2(ALLOWED_DIRECTORY + "test2.jpg", std::ios::binary);\n        file2 << "Image content";\n        file2.close();\n    } catch (const std::exception& e) {\n        std::cerr << "Setup error: " << e.what() << std::endl;\n    }\n    \n    // Test case 1: Valid file\n    std::cout << "Test 1 - Valid file:" << std::endl;\n    std::vector<char> result1 = img("test1.txt");\n    if (!result1.empty()) {\n        std::cout << "Success: " << std::string(result1.begin(), result1.end()) << std::endl;\n    } else {\n        std::cout << "Failed" << std::endl;\n    }\n    \n    // Test case 2: Another valid file\n    std::cout << "\
Test 2 - Valid image file:" << std::endl;\n    std::vector<char> result2 = img("test2.jpg");\n    if (!result2.empty()) {\n        std::cout << "Success: " << std::string(result2.begin(), result2.end()) << std::endl;\n    } else {\n        std::cout << "Failed" << std::endl;\n    }\n    \n    // Test case 3: Path traversal attempt (should be blocked)\n    std::cout << "\
Test 3 - Path traversal attempt:" << std::endl;\n    std::vector<char> result3 = img("../etc/passwd");\n    std::cout << (result3.empty() ? "Blocked (Expected)" : "Success") << std::endl;\n    \n    // Test case 4: Empty parameter\n    std::cout << "\
Test 4 - Empty parameter:" << std::endl;\n    std::vector<char> result4 = img("");\n    std::cout << (result4.empty() ? "Blocked (Expected)" : "Success") << std::endl;\n    \n    // Test case 5: Non-existent file\n    std::cout << "\
Test 5 - Non-existent file:" << std::endl;\n    std::vector<char> result5 = img("nonexistent.txt");\n    std::cout << (result5.empty() ? "Failed (Expected)" : "Success") << std::endl;
    
    return 0;
}
