
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <cstring>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/types.h>
#endif

const std::string SAFE_DIRECTORY = "safe_files";
const std::set<std::string> ALLOWED_EXTENSIONS = {".txt", ".log", ".csv", ".json", ".xml"};
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB

bool isAllowedExtension(const std::string& fileName) {
    for (const auto& ext : ALLOWED_EXTENSIONS) {
        if (fileName.length() >= ext.length()) {
            std::string fileExt = fileName.substr(fileName.length() - ext.length());
            std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
            if (fileExt == ext) {
                return true;
            }
        }
    }
    return false;
}

std::string sanitizeFileName(std::string fileName) {
    // Remove path traversal attempts
    size_t pos;
    while ((pos = fileName.find("..")) != std::string::npos) {
        fileName.erase(pos, 2);
    }
    while ((pos = fileName.find("/")) != std::string::npos) {
        fileName.erase(pos, 1);
    }
    while ((pos = fileName.find("\\\\")) != std::string::npos) {\n        fileName.erase(pos, 1);\n    }\n    return fileName;\n}\n\nbool fileExists(const std::string& path) {\n    struct stat buffer;\n    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));\n}\n\nsize_t getFileSize(const std::string& path) {\n    struct stat buffer;\n    if (stat(path.c_str(), &buffer) == 0) {\n        return buffer.st_size;\n    }\n    return 0;\n}\n\nstd::string retrieveFile(const std::string& fileName) {\n    try {\n        // Input validation\n        if (fileName.empty()) {\n            return "Error: File name cannot be empty";\n        }\n        \n        // Sanitize filename\n        std::string safeName = sanitizeFileName(fileName);\n        \n        // Validate file extension\n        if (!isAllowedExtension(safeName)) {\n            return "Error: File extension not allowed";\n        }\n        \n        // Create safe directory if it doesn't exist\n        #ifdef _WIN32\n        mkdir(SAFE_DIRECTORY.c_str());\n        #else\n        mkdir(SAFE_DIRECTORY.c_str(), 0755);\n        #endif\n        \n        // Build safe file path\n        std::string filePath = SAFE_DIRECTORY + "/" + safeName;\n        \n        // Additional validation: ensure no path traversal in final path\n        if (filePath.find("..") != std::string::npos) {\n            return "Error: Access denied - path traversal detected";\n        }\n        \n        // Check if file exists\n        if (!fileExists(filePath)) {\n            return "Error: File not found";\n        }\n        \n        // Check file size\n        if (getFileSize(filePath) > MAX_FILE_SIZE) {\n            return "Error: File too large";\n        }\n        \n        // Read file content\n        std::ifstream file(filePath);\n        if (!file.is_open()) {\n            return "Error: Cannot open file";\n        }\n        \n        std::stringstream buffer;\n        buffer << file.rdbuf();\n        file.close();\n        \n        return "Success: File retrieved\
" + buffer.str();\n        \n    } catch (const std::exception& e) {\n        return std::string("Error: ") + e.what();\n    }\n}\n\nvoid setupTestFiles() {\n    #ifdef _WIN32\n    mkdir(SAFE_DIRECTORY.c_str());\n    #else\n    mkdir(SAFE_DIRECTORY.c_str(), 0755);\n    #endif\n    \n    std::ofstream testFile(SAFE_DIRECTORY + "/test1.txt");\n    if (testFile.is_open()) {\n        testFile << "This is a test file content.\
Secure file retrieval system.";\n        testFile.close();\n    }\n}\n\nint main() {\n    // Create test files\n    setupTestFiles();\n    \n    // Test Case 1: Valid file retrieval\n    std::cout << "Test 1 - Valid file:" << std::endl;\n    std::cout << retrieveFile("test1.txt") << std::endl;\n    \n    // Test Case 2: Path traversal attempt\n    std::cout << "\
Test 2 - Path traversal attempt:" << std::endl;\n    std::cout << retrieveFile("../../../etc/passwd") << std::endl;\n    \n    // Test Case 3: Invalid extension\n    std::cout << "\
Test 3 - Invalid extension:" << std::endl;\n    std::cout << retrieveFile("malicious.exe") << std::endl;\n    \n    // Test Case 4: Empty filename\n    std::cout << "\
Test 4 - Empty filename:" << std::endl;\n    std::cout << retrieveFile("") << std::endl;\n    \n    // Test Case 5: Non-existent file\n    std::cout << "\
Test 5 - Non-existent file:" << std::endl;\n    std::cout << retrieveFile("nonexistent.txt") << std::endl;
    
    return 0;
}
