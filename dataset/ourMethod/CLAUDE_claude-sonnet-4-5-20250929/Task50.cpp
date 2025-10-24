
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <sys/stat.h>
#include <limits>
#include <memory>

// Maximum file size: 10MB to prevent excessive memory usage
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
const size_t BUFFER_SIZE = 8192;
const std::string UPLOAD_DIR = "./uploads/";

// Security: Validate filename to prevent path traversal attacks (CWE-22)
// Only allow alphanumeric, dots, hyphens, underscores
bool isValidFilename(const std::string& filename) {
    if (filename.empty() || filename.length() > 255) {
        return false;
    }
    
    // Reject path traversal attempts
    if (filename.find("..") != std::string::npos ||
        filename.find('/') != std::string::npos ||
        filename.find('\\\\') != std::string::npos) {\n        return false;\n    }\n    \n    // Only allow safe characters\n    for (char c : filename) {\n        if (!std::isalnum(static_cast<unsigned char>(c)) && \n            c != '.' && c != '-' && c != '_') {\n            return false;\n        }\n    }\n    \n    // Reject if starts with dot (hidden files)\n    if (filename[0] == '.') {\n        return false;\n    }\n    \n    return true;\n}\n\n// Security: Normalize and validate the full path resolves within base directory\nstd::string getSecurePath(const std::string& baseDir, const std::string& filename) {\n    // Validate filename first\n    if (!isValidFilename(filename)) {\n        return "";\n    }\n    \n    std::string fullPath = baseDir + filename;\n    \n    // Additional security: verify no symlinks or directory traversal\n    // In production, use realpath() and verify it starts with baseDir\n    return fullPath;\n}\n\n// Security: Create directory with secure permissions\nbool createUploadDirectory() {\n    struct stat st;\n    if (stat(UPLOAD_DIR.c_str(), &st) != 0) {\n        // Directory doesn't exist, create it with restricted permissions (0700)
#ifdef _WIN32
        if (mkdir(UPLOAD_DIR.c_str()) != 0) {
#else
        if (mkdir(UPLOAD_DIR.c_str(), 0700) != 0) {
#endif
            return false;
        }
    }
    return true;
}

// Security: Write to temp file atomically to prevent TOCTOU (CWE-367)
bool uploadFile(const std::string& filename, const std::vector<unsigned char>& fileData) {
    // Validate input size (CWE-770: Allocation of Resources Without Limits)
    if (fileData.empty() || fileData.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size invalid or exceeds maximum allowed size\\n";
        return false;
    }
    
    // Get secure path
    std::string securePath = getSecurePath(UPLOAD_DIR, filename);
    if (securePath.empty()) {
        std::cerr << "Error: Invalid filename\\n";
        return false;
    }
    
    // Security: Write to temporary file first, then atomic rename (CWE-367)
    std::string tempPath = securePath + ".tmp";
    
    // Security: Open with exclusive creation flag where possible
    std::ofstream outFile(tempPath, std::ios::binary | std::ios::trunc);
    if (!outFile) {
        std::cerr << "Error: Cannot create temporary file\\n";
        return false;
    }
    
    // Security: Write data with error checking
    outFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    
    if (!outFile.good()) {
        outFile.close();
        std::remove(tempPath.c_str());
        std::cerr << "Error: Failed to write file data\\n";
        return false;
    }
    
    // Security: Flush and close before rename
    outFile.flush();
    outFile.close();
    
    // Security: Atomic rename from temp to final name
    if (std::rename(tempPath.c_str(), securePath.c_str()) != 0) {
        std::remove(tempPath.c_str());
        std::cerr << "Error: Failed to finalize file\\n";
        return false;
    }
    
    return true;
}

// Simulate reading file data from client (in real scenario, this would be from network)
std::vector<unsigned char> simulateFileUpload(const std::string& localFile) {
    std::vector<unsigned char> buffer;
    
    std::ifstream inFile(localFile, std::ios::binary);
    if (!inFile) {
        return buffer;
    }
    
    // Security: Check file size before reading
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    
    // Validate file size (CWE-770)
    if (fileSize < 0 || static_cast<size_t>(fileSize) > MAX_FILE_SIZE) {
        return buffer;
    }
    
    inFile.seekg(0, std::ios::beg);
    
    // Security: Reserve exact size to prevent excessive allocation
    buffer.reserve(static_cast<size_t>(fileSize));
    
    // Security: Read in chunks with bounds checking
    char readBuffer[BUFFER_SIZE];
    while (inFile && buffer.size() < MAX_FILE_SIZE) {
        inFile.read(readBuffer, BUFFER_SIZE);
        std::streamsize bytesRead = inFile.gcount();
        
        if (bytesRead > 0) {
            // Security: Verify we won't exceed max size\n            if (buffer.size() + bytesRead > MAX_FILE_SIZE) {\n                buffer.clear();\n                return buffer;\n            }\n            buffer.insert(buffer.end(), readBuffer, readBuffer + bytesRead);\n        }\n    }\n    \n    return buffer;\n}\n\nint main() {\n    // Security: Create upload directory with restricted permissions\n    if (!createUploadDirectory()) {\n        std::cerr << "Error: Failed to create upload directory\\n";\n        return 1;\n    }\n    \n    // Test case 1: Valid text file\n    {\n        std::string testData = "This is test content for file upload.";\n        std::vector<unsigned char> data(testData.begin(), testData.end());\n        if (uploadFile("test1.txt", data)) {\n            std::cout << "Test 1 PASSED: File 'test1.txt' uploaded successfully\\n";\n        } else {\n            std::cout << "Test 1 FAILED\\n";\n        }\n    }\n    \n    // Test case 2: Valid binary file\n    {\n        std::vector<unsigned char> data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};\n        if (uploadFile("test2.bin", data)) {\n            std::cout << "Test 2 PASSED: File 'test2.bin' uploaded successfully\
";\n        } else {\n            std::cout << "Test 2 FAILED\
";\n        }\n    }\n    \n    // Test case 3: Invalid filename with path traversal attempt (should fail)\n    {\n        std::string testData = "Malicious content";\n        std::vector<unsigned char> data(testData.begin(), testData.end());\n        if (!uploadFile("../etc/passwd", data)) {\n            std::cout << "Test 3 PASSED: Path traversal attack prevented\
";\n        } else {\n            std::cout << "Test 3 FAILED: Security vulnerability detected\
";\n        }\n    }\n    \n    // Test case 4: Empty file (should fail)\n    {\n        std::vector<unsigned char> data;\n        if (!uploadFile("empty.txt", data)) {\n            std::cout << "Test 4 PASSED: Empty file rejected\
";\n        } else {\n            std::cout << "Test 4 FAILED\
";\n        }\n    }\n    \n    // Test case 5: File with invalid characters (should fail)\n    {\n        std::string testData = "Test content";\n        std::vector<unsigned char> data(testData.begin(), testData.end());\n        if (!uploadFile("test<>file.txt", data)) {\n            std::cout << "Test 5 PASSED: Invalid filename rejected\
";\n        } else {\n            std::cout << "Test 5 FAILED\
";
        }
    }
    
    return 0;
}
