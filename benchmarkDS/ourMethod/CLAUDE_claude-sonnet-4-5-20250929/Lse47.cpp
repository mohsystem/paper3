
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <limits.h>

const std::string BASE_DIR = "images";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

/**
 * Validates and safely resolves a file path within the base directory.
 * Returns empty string if the path is invalid or outside the base directory.
 */
std::string validateAndResolvePath(const std::string& filename) {
    if (filename.empty()) {
        return "";
    }
    
    // Reject path traversal patterns and directory separators
    if (filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos ||
        filename.find("\\\\") != std::string::npos) {\n        return "";\n    }\n    \n    // Construct the full path\n    std::string fullPath = BASE_DIR + "/" + filename;\n    \n    // Resolve to canonical path\n    char resolvedPath[PATH_MAX];\n    if (realpath(fullPath.c_str(), resolvedPath) == NULL) {\n        return "";\n    }\n    \n    // Resolve base directory\n    char resolvedBase[PATH_MAX];\n    if (realpath(BASE_DIR.c_str(), resolvedBase) == NULL) {\n        return "";\n    }\n    \n    // Ensure the resolved path starts with the base directory\n    std::string resolvedPathStr(resolvedPath);\n    std::string resolvedBaseStr(resolvedBase);\n    \n    if (resolvedPathStr.find(resolvedBaseStr) != 0) {\n        return "";\n    }\n    \n    return resolvedPathStr;\n}\n\n/**\n * Handles image file requests with secure path validation.\n */\nstd::vector<unsigned char> serveImage(const std::string& filename) {\n    std::vector<unsigned char> empty;\n    \n    std::string validPath = validateAndResolvePath(filename);\n    if (validPath.empty()) {\n        return empty;\n    }\n    \n    // Open file with O_RDONLY and O_NOFOLLOW to prevent symlink following\n    int fd = open(validPath.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);\n    if (fd < 0) {\n        return empty;\n    }\n    \n    // Validate the opened file descriptor\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        close(fd);\n        return empty;\n    }\n    \n    // Check if it's a regular file (not directory or symlink)\n    if (!S_ISREG(st.st_mode)) {\n        close(fd);\n        return empty;\n    }\n    \n    // Check file size\n    if (st.st_size < 0 || static_cast<size_t>(st.st_size) > MAX_FILE_SIZE) {\n        close(fd);\n        return empty;\n    }\n    \n    // Read file contents\n    std::vector<unsigned char> buffer(st.st_size);\n    ssize_t bytesRead = read(fd, buffer.data(), st.st_size);\n    close(fd);\n    \n    if (bytesRead != st.st_size) {\n        return empty;\n    }\n    \n    return buffer;\n}\n\nint main() {\n    // Create images directory if it doesn't exist\n    mkdir(BASE_DIR.c_str(), 0755);\n    \n    // Test case setup - create test files\n    {\n        std::ofstream f1(BASE_DIR + "/test1.jpg", std::ios::binary);\n        f1 << "test image 1";\n        \n        std::ofstream f2(BASE_DIR + "/test2.png", std::ios::binary);\n        f2 << "test image 2";\n        \n        std::ofstream f3(BASE_DIR + "/valid.gif", std::ios::binary);\n        f3 << "test image 3";\n    }\n    \n    // Test cases\n    std::cout << "Test 1 - Valid file:" << std::endl;\n    std::vector<unsigned char> result1 = serveImage("test1.jpg");\n    std::cout << (!result1.empty() ? "SUCCESS - File served" : "FAILED") << std::endl;\n    \n    std::cout << "\
Test 2 - Another valid file:" << std::endl;\n    std::vector<unsigned char> result2 = serveImage("test2.png");\n    std::cout << (!result2.empty() ? "SUCCESS - File served" : "FAILED") << std::endl;\n    \n    std::cout << "\
Test 3 - Non-existent file:" << std::endl;\n    std::vector<unsigned char> result3 = serveImage("nonexistent.jpg");\n    std::cout << (result3.empty() ? "SUCCESS - Rejected" : "FAILED") << std::endl;\n    \n    std::cout << "\
Test 4 - Path traversal attempt:" << std::endl;\n    std::vector<unsigned char> result4 = serveImage("../etc/passwd");\n    std::cout << (result4.empty() ? "SUCCESS - Rejected" : "FAILED") << std::endl;\n    \n    std::cout << "\
Test 5 - Path with slash:" << std::endl;\n    std::vector<unsigned char> result5 = serveImage("subdir/file.jpg");\n    std::cout << (result5.empty() ? "SUCCESS - Rejected" : "FAILED") << std::endl;
    
    return 0;
}
