
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// Maximum file size: 10MB
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;

// Upload directory name
const char* UPLOAD_DIR = "uploads";

// Allowed file extensions
const std::vector<std::string> ALLOWED_EXTENSIONS = {
    "txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"
};

// Forward declarations
std::string sanitizeFilename(const std::string& filename);
std::string getFileExtension(const std::string& filename);
std::string generateUniqueFilename(const std::string& originalFilename);
bool isExtensionAllowed(const std::string& extension);
void createUploadDirectory();
std::string normalizeAndValidatePath(const std::string& basePath, const std::string& filename);

// Secure file upload handler
std::string uploadFile(const std::string& filename, const std::vector<unsigned char>& fileContent) {
    // Input validation: reject empty filename
    if (filename.empty()) {
        throw std::invalid_argument("Invalid filename");
    }
    
    // Sanitize filename to prevent path traversal
    std::string sanitizedFilename = sanitizeFilename(filename);
    if (sanitizedFilename.empty()) {
        throw std::invalid_argument("Invalid filename after sanitization");
    }
    
    // Validate file extension
    std::string extension = getFileExtension(sanitizedFilename);
    if (!isExtensionAllowed(extension)) {
        throw std::invalid_argument("File type not allowed: " + extension);
    }
    
    // Validate file size
    if (fileContent.size() > MAX_FILE_SIZE) {
        throw std::invalid_argument("File size exceeds maximum allowed size");
    }
    
    // Create uploads directory with restrictive permissions
    createUploadDirectory();
    
    // Generate unique filename
    std::string uniqueFilename = generateUniqueFilename(sanitizedFilename);
    
    // Normalize and validate target path to prevent traversal
    std::string targetPath = normalizeAndValidatePath(UPLOAD_DIR, uniqueFilename);
    
    // Open file with O_CREAT|O_EXCL|O_WRONLY for atomic creation
    // O_EXCL prevents TOCTOU by failing if file exists
    // O_NOFOLLOW prevents following symlinks
    int fd = open(targetPath.c_str(), 
                  O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 
                  S_IRUSR | S_IWUSR);  // 0600 permissions
    
    if (fd < 0) {
        throw std::runtime_error("Failed to create file: " + std::string(strerror(errno)));
    }
    
    // Validate opened file descriptor
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        throw std::runtime_error("Failed to stat file");
    }
    
    // Ensure it's a regular file\n    if (!S_ISREG(st.st_mode)) {\n        close(fd);\n        unlink(targetPath.c_str());\n        throw std::runtime_error("Target is not a regular file");\n    }\n    \n    // Write content to file\n    ssize_t bytesWritten = write(fd, fileContent.data(), fileContent.size());\n    if (bytesWritten < 0 || static_cast<size_t>(bytesWritten) != fileContent.size()) {\n        close(fd);\n        unlink(targetPath.c_str());\n        throw std::runtime_error("Failed to write file content");\n    }\n    \n    // Flush to disk\n    if (fsync(fd) != 0) {\n        close(fd);\n        unlink(targetPath.c_str());\n        throw std::runtime_error("Failed to sync file to disk");\n    }\n    \n    close(fd);\n    return uniqueFilename;\n}\n\nstd::string sanitizeFilename(const std::string& filename) {\n    std::string sanitized = filename;\n    \n    // Remove null bytes, path separators\n    sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\\0'), sanitized.end());\n    sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '/'), sanitized.end());\n    sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\\\\'), sanitized.end());\n    \n    // Remove leading/trailing dots and spaces\n    size_t start = sanitized.find_first_not_of(". ");\n    size_t end = sanitized.find_last_not_of(". ");\n    \n    if (start == std::string::npos || end == std::string::npos) {\n        return "";\n    }\n    \n    sanitized = sanitized.substr(start, end - start + 1);\n    \n    // Limit length\n    if (sanitized.length() > 255) {\n        sanitized = sanitized.substr(0, 255);\n    }\n    \n    return sanitized;\n}\n\nstd::string getFileExtension(const std::string& filename) {\n    size_t pos = filename.find_last_of('.');\n    if (pos != std::string::npos && pos < filename.length() - 1) {\n        return filename.substr(pos + 1);\n    }\n    return "";\n}\n\nbool isExtensionAllowed(const std::string& extension) {\n    std::string lowerExt = extension;\n    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);\n    \n    return std::find(ALLOWED_EXTENSIONS.begin(), ALLOWED_EXTENSIONS.end(), lowerExt) \n           != ALLOWED_EXTENSIONS.end();\n}\n\nstd::string generateUniqueFilename(const std::string& originalFilename) {\n    // Generate random hex string using secure random\n    std::random_device rd;\n    std::mt19937 gen(rd());\n    std::uniform_int_distribution<> dis(0, 255);\n    \n    std::stringstream ss;\n    for (int i = 0; i < 16; ++i) {\n        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);\n    }\n    \n    std::string extension = getFileExtension(originalFilename);\n    std::string baseName = originalFilename.substr(0, originalFilename.find_last_of('.'));\n    \n    if (baseName.length() > 50) {\n        baseName = baseName.substr(0, 50);\n    }\n    \n    return baseName + "_" + ss.str() + "." + extension;\n}\n\nvoid createUploadDirectory() {\n    struct stat st;\n    if (stat(UPLOAD_DIR, &st) != 0) {\n        // Directory doesn't exist, create it with restrictive permissions
        if (mkdir(UPLOAD_DIR, S_IRWXU) != 0) {  // 0700 permissions
            throw std::runtime_error("Failed to create upload directory");
        }
    }
}

std::string normalizeAndValidatePath(const std::string& basePath, const std::string& filename) {
    // Construct full path
    std::string fullPath = std::string(basePath) + "/" + filename;
    
    // Check for path traversal patterns
    if (fullPath.find("..") != std::string::npos) {
        throw std::invalid_argument("Path traversal attempt detected");
    }
    
    // Ensure path starts with base directory
    if (fullPath.find(basePath) != 0) {
        throw std::invalid_argument("Invalid path");
    }
    
    return fullPath;
}

int main() {
    // Test case 1: Valid text file
    try {
        std::string content1 = "Test file content";
        std::vector<unsigned char> data1(content1.begin(), content1.end());
        std::string result1 = uploadFile("test.txt", data1);
        std::cout << "Test 1 passed: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Path traversal attempt (should fail)
    try {
        std::string content2 = "Malicious content";
        std::vector<unsigned char> data2(content2.begin(), content2.end());
        std::string result2 = uploadFile("../../../etc/passwd.txt", data2);
        std::cout << "Test 2 failed: Should have rejected path traversal" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 passed: Rejected path traversal - " << e.what() << std::endl;
    }
    
    // Test case 3: Invalid file extension (should fail)
    try {
        std::string content3 = "<?php system($_GET['cmd']); ?>";
        std::vector<unsigned char> data3(content3.begin(), content3.end());
        std::string result3 = uploadFile("shell.php", data3);
        std::cout << "Test 3 failed: Should have rejected .php extension" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 passed: Rejected invalid extension - " << e.what() << std::endl;
    }
    
    // Test case 4: Valid PDF file
    try {
        std::string content4 = "%PDF-1.4 test";
        std::vector<unsigned char> data4(content4.begin(), content4.end());
        std::string result4 = uploadFile("document.pdf", data4);
        std::cout << "Test 4 passed: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: Empty filename (should fail)
    try {
        std::string content5 = "Content";
        std::vector<unsigned char> data5(content5.begin(), content5.end());
        std::string result5 = uploadFile("", data5);
        std::cout << "Test 5 failed: Should have rejected empty filename" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 passed: Rejected empty filename - " << e.what() << std::endl;
    }
    
    return 0;
}
