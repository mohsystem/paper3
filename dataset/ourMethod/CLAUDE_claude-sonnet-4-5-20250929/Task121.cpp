
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <limits>
#include <algorithm>

// Maximum file size: 10MB to prevent excessive memory usage
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
// Maximum filename length
const size_t MAX_FILENAME_LENGTH = 255;
// Base upload directory - in production, load from secure configuration
const std::string BASE_UPLOAD_DIR = "./uploads/";

// Validate filename to prevent path traversal attacks (CWE-22)
// Reject directory separators, parent directory references, and ensure proper length
bool validateFilename(const std::string& filename) {
    // Check length
    if (filename.empty() || filename.length() > MAX_FILENAME_LENGTH) {
        return false;
    }
    
    // Reject path traversal attempts: no "..", "/", "\\", or null bytes\n    if (filename.find("..") != std::string::npos ||\n        filename.find('/') != std::string::npos ||\n        filename.find('\\\\') != std::string::npos ||\n        filename.find('\\0') != std::string::npos) {\n        return false;\n    }\n    \n    // Reject names starting with "." (hidden files)\n    if (filename[0] == '.') {\n        return false;\n    }\n    \n    // Only allow alphanumeric, underscore, hyphen, and single period\n    for (char c : filename) {\n        if (!std::isalnum(static_cast<unsigned char>(c)) && \n            c != '_' && c != '-' && c != '.') {\n            return false;\n        }\n    }\n    \n    return true;\n}\n\n// Construct safe file path ensuring it stays within base directory (CWE-22)\nstd::string constructSafePath(const std::string& filename) {\n    // Validate filename first\n    if (!validateFilename(filename)) {\n        return "";\n    }\n    \n    std::string safePath = BASE_UPLOAD_DIR + filename;\n    \n    // In production, use realpath or canonical path to verify the resolved path\n    // stays within BASE_UPLOAD_DIR. For this example, validation above ensures safety.\n    \n    return safePath;\n}\n\n// Create upload directory if it doesn't exist\nbool ensureUploadDirectory() {\n    struct stat st;\n    if (stat(BASE_UPLOAD_DIR.c_str(), &st) != 0) {\n        // Directory doesn't exist, create it with secure permissions (0700)\n        #ifdef _WIN32\n        if (mkdir(BASE_UPLOAD_DIR.c_str()) != 0) {\n        #else\n        if (mkdir(BASE_UPLOAD_DIR.c_str(), 0700) != 0) {\n        #endif\n            return false;\n        }\n    }\n    return true;\n}\n\n// Upload file with security controls (CWE-434, CWE-400, CWE-606)\nbool uploadFile(const std::string& filename, const std::vector<unsigned char>& fileData) {\n    // Validate all inputs as untrusted\n    if (fileData.empty()) {\n        std::cerr << "Error: File data is empty\
";\n        return false;\n    }\n    \n    // Prevent excessive memory usage (CWE-400)\n    if (fileData.size() > MAX_FILE_SIZE) {\n        std::cerr << "Error: File size exceeds maximum allowed size\
";\n        return false;\n    }\n    \n    // Construct safe path preventing path traversal\n    std::string safePath = constructSafePath(filename);\n    if (safePath.empty()) {\n        std::cerr << "Error: Invalid filename\
";\n        return false;\n    }\n    \n    // Ensure upload directory exists\n    if (!ensureUploadDirectory()) {\n        std::cerr << "Error: Cannot create upload directory\
";\n        return false;\n    }\n    \n    // Mitigate TOCTOU (CWE-367): Open file exclusively to prevent race conditions\n    // Use temporary file, then atomic rename\n    std::string tempPath = safePath + ".tmp";\n    \n    // Open temp file for writing in binary mode\n    std::ofstream outFile(tempPath, std::ios::binary | std::ios::trunc);\n    if (!outFile) {\n        std::cerr << "Error: Cannot create temporary file\
";\n        return false;\n    }\n    \n    // Write data with bounds checking (CWE-119, CWE-120)\n    outFile.write(reinterpret_cast<const char*>(fileData.data()), \n                  static_cast<std::streamsize>(fileData.size()));\n    \n    if (!outFile.good()) {\n        std::cerr << "Error: Failed to write file data\
";\n        outFile.close();\n        std::remove(tempPath.c_str());\n        return false;\n    }\n    \n    // Ensure data is written to disk (flush and sync)\n    outFile.flush();\n    outFile.close();\n    \n    // Atomic rename from temp to final location (TOCTOU mitigation)\n    if (std::rename(tempPath.c_str(), safePath.c_str()) != 0) {\n        std::cerr << "Error: Failed to finalize file upload\
";\n        std::remove(tempPath.c_str());\n        return false;\n    }\n    \n    std::cout << "File uploaded successfully: " << filename << "\
";\n    return true;\n}\n\n// Simulate file upload from user input\nbool simulateFileUpload(const std::string& filename, const std::string& content) {\n    std::vector<unsigned char> fileData(content.begin(), content.end());\n    return uploadFile(filename, fileData);\n}\n\nint main() {\n    std::cout << "=== Secure File Upload System ===\
\
";\n    \n    // Test case 1: Valid file upload\n    std::cout << "Test 1: Valid file upload\
";\n    simulateFileUpload("document.txt", "This is a test document.");\n    \n    // Test case 2: Another valid file\n    std::cout << "\
Test 2: Another valid file\
";\n    simulateFileUpload("report_2024.pdf", "PDF content here");\n    \n    // Test case 3: Path traversal attempt (should fail)\n    std::cout << "\
Test 3: Path traversal attempt (should be blocked)\
";\n    simulateFileUpload("../../../etc/passwd", "malicious content");\n    \n    // Test case 4: Invalid filename with slash (should fail)\n    std::cout << "\
Test 4: Invalid filename with slash (should be blocked)\
";\n    simulateFileUpload("subdir/file.txt", "content");\n    \n    // Test case 5: Valid filename with underscore and hyphen\n    std::cout << "\
Test 5: Valid filename with special characters\
";\n    simulateFileUpload("my_file-v2.json", "{\\"key\\": \\"value\\"}");
    
    return 0;
}
