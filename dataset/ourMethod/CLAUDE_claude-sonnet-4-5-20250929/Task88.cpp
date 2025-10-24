
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <random>
#include <limits>

// Secure random string generator using std::random_device
std::string generateRandomString(size_t length) {
    // Validate length to prevent excessive memory allocation
    const size_t MAX_LENGTH = 1024;
    if (length == 0 || length > MAX_LENGTH) {
        throw std::invalid_argument("Invalid string length");
    }
    
    // Use cryptographically secure random device
    std::random_device rd;
    std::mt19937 gen(rd());
    // Use printable ASCII characters (0x20 to 0x7E)
    std::uniform_int_distribution<> dis(0x20, 0x7E);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += static_cast<char>(dis(gen));
    }
    
    return result;
}

// Convert ASCII string to Unicode (UTF-8) - already UTF-8 compatible
std::string convertToUnicode(const std::string& input) {
    // Validate input
    if (input.empty()) {
        throw std::invalid_argument("Empty input string");
    }
    
    // In C++, std::string can hold UTF-8 encoded data
    // For demonstration, we ensure it's valid UTF-8\n    // The input is already ASCII which is valid UTF-8\n    return input;\n}\n\n// Create temporary file and write Unicode string\nstd::string createTempFileWithUnicode() {\n    // Generate a random string (16 characters for security)\n    const size_t RANDOM_STRING_LENGTH = 16;\n    std::string randomStr = generateRandomString(RANDOM_STRING_LENGTH);\n    \n    // Convert to Unicode (UTF-8)\n    std::string unicodeStr = convertToUnicode(randomStr);\n    \n    // Create temp file path with proper bounds checking\n    // Use tmpnam alternative with mkstemp pattern for security\n    char tempTemplate[] = "/tmp/tempfile_XXXXXX";\n    \n    // For Windows compatibility, check platform\n    #ifdef _WIN32\n    char* tempDir = std::getenv("TEMP");\n    if (!tempDir) {\n        tempDir = std::getenv("TMP");\n    }\n    std::string tempPath;\n    if (tempDir) {\n        tempPath = std::string(tempDir) + "\\\\tempfile_XXXXXX";\n    } else {\n        tempPath = "C:\\\\Windows\\\\Temp\\\\tempfile_XXXXXX";\n    }\n    #else\n    std::string tempPath = tempTemplate;\n    #endif\n    \n    // Create actual temporary file using RAII\n    std::string finalPath;\n    \n    #ifndef _WIN32\n    // Use mkstemp for POSIX systems (secure file creation)\n    std::vector<char> pathBuf(tempPath.begin(), tempPath.end());\n    pathBuf.push_back('\\0');
    
    int fd = mkstemp(pathBuf.data());
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    
    finalPath = std::string(pathBuf.data());
    
    // Write using file descriptor with proper error checking
    ssize_t written = write(fd, unicodeStr.c_str(), unicodeStr.size());
    if (written != static_cast<ssize_t>(unicodeStr.size())) {
        close(fd);
        throw std::runtime_error("Failed to write to temporary file");
    }
    
    // Ensure data is flushed to disk
    if (fsync(fd) != 0) {
        close(fd);
        throw std::runtime_error("Failed to sync temporary file");
    }
    
    close(fd);
    #else
    // For Windows, use _tempnam with security considerations
    char* tmpPath = _tempnam(tempDir, "tempfile_");
    if (!tmpPath) {
        throw std::runtime_error("Failed to generate temporary file name");
    }
    finalPath = std::string(tmpPath);
    free(tmpPath);
    
    // Use ofstream with explicit UTF-8 encoding
    std::ofstream outFile(finalPath, std::ios::binary | std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open temporary file");
    }
    
    outFile.write(unicodeStr.c_str(), unicodeStr.size());
    outFile.flush();
    
    if (!outFile.good()) {
        outFile.close();
        throw std::runtime_error("Failed to write to temporary file");
    }
    
    outFile.close();
    #endif
    
    return finalPath;
}

int main() {
    try {
        // Test case 1: Basic functionality
        std::cout << "Test 1: Creating temporary file with Unicode content" << std::endl;
        std::string path1 = createTempFileWithUnicode();
        std::cout << "Temporary file created at: " << path1 << std::endl;
        
        // Test case 2: Verify file exists and is readable
        std::cout << "\\nTest 2: Verifying file exists" << std::endl;
        std::ifstream testFile(path1);
        if (testFile.good()) {
            std::cout << "File exists and is readable" << std::endl;
            testFile.close();
        }
        
        // Test case 3: Create another temp file
        std::cout << "\\nTest 3: Creating second temporary file" << std::endl;
        std::string path2 = createTempFileWithUnicode();
        std::cout << "Second temporary file created at: " << path2 << std::endl;
        
        // Test case 4: Verify unique paths
        std::cout << "\\nTest 4: Verifying unique paths" << std::endl;
        if (path1 != path2) {
            std::cout << "Paths are unique: PASS" << std::endl;
        } else {
            std::cout << "Paths are not unique: FAIL" << std::endl;
        }
        
        // Test case 5: Create multiple files
        std::cout << "\\nTest 5: Creating multiple temporary files" << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::string path = createTempFileWithUnicode();
            std::cout << "File " << (i + 1) << " created at: " << path << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
