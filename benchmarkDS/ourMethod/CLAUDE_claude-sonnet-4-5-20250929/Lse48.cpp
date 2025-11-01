
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Secure base directory for serving images - must be set to a trusted directory
const std::string BASE_DIR = "./images";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

// Helper function to normalize and validate paths
std::string normalizePath(const std::string& path) {
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved) == nullptr) {
        throw std::runtime_error("Failed to resolve path");
    }
    return std::string(resolved);
}

// Helper function to check if path is within base directory
bool isPathWithinBase(const std::string& fullPath, const std::string& basePath) {
    return fullPath.compare(0, basePath.length(), basePath) == 0;
}

std::vector<unsigned char> img(const std::string& fileParam) {
    // Input validation: reject empty file parameter
    if (fileParam.empty()) {
        throw std::invalid_argument("File parameter is required");
    }

    // Get the absolute base directory path
    char baseResolved[PATH_MAX];
    if (realpath(BASE_DIR.c_str(), baseResolved) == nullptr) {
        throw std::runtime_error("Base directory does not exist");
    }
    std::string baseDir(baseResolved);

    // Construct the requested path
    std::string requestedPath = BASE_DIR + "/" + fileParam;

    // Normalize and resolve the requested path
    // This prevents path traversal attacks
    std::string resolvedPath;
    try {
        resolvedPath = normalizePath(requestedPath);
    } catch (...) {
        throw std::runtime_error("File not found");
    }

    // Security check: ensure the resolved path is within BASE_DIR
    // This prevents directory traversal attacks like ../../../etc/passwd
    if (!isPathWithinBase(resolvedPath, baseDir)) {
        throw std::runtime_error("Access denied: path traversal attempt detected");
    }

    // Open file descriptor with secure flags - O_NOFOLLOW prevents symlink following
    int fd = open(resolvedPath.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file");
    }

    // Validate the opened file descriptor using fstat
    struct stat fileStat;
    if (fstat(fd, &fileStat) != 0) {
        close(fd);
        throw std::runtime_error("Failed to stat file");
    }

    // Validate that it's a regular file (not a directory or symlink)\n    if (!S_ISREG(fileStat.st_mode)) {\n        close(fd);\n        throw std::runtime_error("Access denied: not a regular file");\n    }\n\n    // Check file size before reading to prevent memory exhaustion\n    if (fileStat.st_size > static_cast<off_t>(MAX_FILE_SIZE)) {\n        close(fd);\n        throw std::runtime_error("File too large");\n    }\n\n    // Allocate buffer for file content\n    std::vector<unsigned char> buffer(fileStat.st_size);\n\n    // Read file content securely using the validated file descriptor\n    ssize_t totalRead = 0;\n    while (totalRead < fileStat.st_size) {\n        ssize_t bytesRead = read(fd, buffer.data() + totalRead, \n                                 fileStat.st_size - totalRead);\n        if (bytesRead < 0) {\n            close(fd);\n            throw std::runtime_error("Error reading file");\n        }\n        if (bytesRead == 0) {\n            break; // EOF\n        }\n        totalRead += bytesRead;\n    }\n\n    close(fd);\n    return buffer;\n}\n\nint main() {\n    // Create base directory if it doesn't exist
    mkdir(BASE_DIR.c_str(), 0700);

    std::cout << "Test Case 1: Valid image file" << std::endl;
    try {
        // Create a test file
        std::string testPath = BASE_DIR + "/test1.jpg";
        std::ofstream testFile(testPath, std::ios::binary);
        unsigned char testData[] = {0xFF, 0xD8, 0xFF, 0xE0};
        testFile.write(reinterpret_cast<char*>(testData), sizeof(testData));
        testFile.close();
        
        std::vector<unsigned char> result1 = img("test1.jpg");
        std::cout << "Success: Read " << result1.size() << " bytes" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << "\\nTest Case 2: Path traversal attempt (should fail)" << std::endl;
    try {
        std::vector<unsigned char> result2 = img("../../../etc/passwd");
        std::cout << "Error: Should have been blocked" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Success: " << e.what() << std::endl;
    }

    std::cout << "\\nTest Case 3: Empty file parameter (should fail)" << std::endl;
    try {
        std::vector<unsigned char> result3 = img("");
        std::cout << "Error: Should have been blocked" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Success: " << e.what() << std::endl;
    }

    std::cout << "\\nTest Case 4: Non-existent file (should fail)" << std::endl;
    try {
        std::vector<unsigned char> result4 = img("nonexistent.jpg");
        std::cout << "Error: Should have failed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Success: " << e.what() << std::endl;
    }

    std::cout << "\\nTest Case 5: Path with null bytes (should fail)" << std::endl;
    try {
        std::string malicious = "test";
        malicious += '\\0';
        malicious += "hidden";
        std::vector<unsigned char> result5 = img(malicious);
        std::cout << "Error: Should have been blocked" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Success: " << e.what() << std::endl;
    }

    return 0;
}
