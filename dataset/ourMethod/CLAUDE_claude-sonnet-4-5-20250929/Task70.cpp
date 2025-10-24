
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <limits>

// Maximum file size to prevent excessive memory usage (10 MB)
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;

// Function to normalize and validate file path
// Returns empty string if path is invalid or outside allowed directory
std::string normalizePath(const std::string& inputPath, const std::string& baseDir) {
    // Basic input validation - reject empty paths
    if (inputPath.empty()) {
        return "";
    }
    
    // Reject paths with null bytes (path injection attempt)
    if (inputPath.find('\\0') != std::string::npos) {
        return "";
    }
    
    // Reject absolute paths and paths attempting traversal
    if (inputPath[0] == '/' || inputPath.find("..") != std::string::npos) {
        return "";
    }
    
    // Construct full path within base directory
    std::string fullPath = baseDir + "/" + inputPath;
    
    return fullPath;
}

// Function to read file content securely
// Returns true on success, false on failure
bool readFileContent(const std::string& filePath, std::string& content) {
    // Clear output parameter
    content.clear();
    
    // Open file in binary mode with error checking
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file" << std::endl;
        return false;
    }
    
    // Get file size and validate it's within limits\n    std::streamsize fileSize = file.tellg();\n    if (fileSize < 0) {\n        std::cerr << "Error: Cannot determine file size" << std::endl;\n        file.close();\n        return false;\n    }\n    \n    // Check file size against maximum to prevent excessive memory usage\n    if (static_cast<size_t>(fileSize) > MAX_FILE_SIZE) {\n        std::cerr << "Error: File too large (max " << MAX_FILE_SIZE << " bytes)" << std::endl;\n        file.close();\n        return false;\n    }\n    \n    // Return to beginning of file\n    file.seekg(0, std::ios::beg);\n    if (!file.good()) {\n        std::cerr << "Error: Cannot seek to beginning of file" << std::endl;\n        file.close();\n        return false;\n    }\n    \n    // Reserve exact space needed to prevent reallocations\n    content.reserve(static_cast<size_t>(fileSize));\n    \n    // Read file content with bounds checking\n    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());\n    \n    // Verify read was successful\n    if (file.bad()) {\n        std::cerr << "Error: Failed to read file content" << std::endl;\n        content.clear();\n        file.close();\n        return false;\n    }\n    \n    file.close();\n    return true;\n}\n\nint main(int argc, char* argv[]) {\n    // Validate command line arguments count\n    if (argc != 2) {\n        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program") << " <filepath>" << std::endl;\n        return 1;\n    }\n    \n    // Validate argv pointer is not null (defensive check)\n    if (argv == nullptr || argv[1] == nullptr) {\n        std::cerr << "Error: Invalid arguments" << std::endl;\n        return 1;\n    }\n    \n    // Get input path and validate it's not too long
    std::string inputPath(argv[1]);
    if (inputPath.length() > 4096) {
        std::cerr << "Error: Path too long" << std::endl;
        return 1;
    }
    
    // Define base directory for file operations (current directory)
    std::string baseDir = ".";
    
    // Normalize and validate the path
    std::string validatedPath = normalizePath(inputPath, baseDir);
    if (validatedPath.empty()) {
        std::cerr << "Error: Invalid file path" << std::endl;
        return 1;
    }
    
    // Read file content
    std::string content;
    if (!readFileContent(validatedPath, content)) {
        return 1;
    }
    
    // Output content to stdout
    std::cout << content;
    
    return 0;
}

// Test cases (compile with -DRUN_TESTS to enable)
#ifdef RUN_TESTS
void runTests() {
    std::cout << "Running test cases..." << std::endl;
    
    // Test 1: Valid file
    std::ofstream test1("test1.txt");
    test1 << "Hello, World!";
    test1.close();
    
    std::string content;
    if (readFileContent("./test1.txt", content)) {
        std::cout << "Test 1 passed: " << content << std::endl;
    }
    
    // Test 2: Non-existent file
    if (!readFileContent("./nonexistent.txt", content)) {
        std::cout << "Test 2 passed: Correctly handled non-existent file" << std::endl;
    }
    
    // Test 3: Path traversal attempt
    std::string maliciousPath = normalizePath("../etc/passwd", ".");
    if (maliciousPath.empty()) {
        std::cout << "Test 3 passed: Blocked path traversal" << std::endl;
    }
    
    // Test 4: Empty path
    std::string emptyPath = normalizePath("", ".");
    if (emptyPath.empty()) {
        std::cout << "Test 4 passed: Rejected empty path" << std::endl;
    }
    
    // Test 5: Valid multiline content
    std::ofstream test5("test5.txt");
    test5 << "Line 1\\nLine 2\\nLine 3";
    test5.close();
    
    if (readFileContent("./test5.txt", content)) {
        std::cout << "Test 5 passed: Read multiline content" << std::endl;
    }
}
#endif
