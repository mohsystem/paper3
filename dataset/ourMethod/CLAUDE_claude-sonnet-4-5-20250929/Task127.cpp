
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>

// Normalize and validate path to prevent directory traversal
// Returns empty string if path is invalid or escapes base directory
std::string normalizePath(const std::string& basePath, const std::string& userPath) {
    // Reject empty paths
    if (userPath.empty() || basePath.empty()) {
        return "";
    }
    
    // Reject paths with null bytes
    if (userPath.find('\\0') != std::string::npos || basePath.find('\\0') != std::string::npos) {
        return "";
    }
    
    // Reject absolute paths that don't start with base\n    if (!userPath.empty() && userPath[0] == '/') {\n        if (userPath.find(basePath) != 0) {\n            return "";\n        }\n    }\n    \n    // Construct full path\n    std::string fullPath = basePath;\n    if (fullPath.back() != '/') {\n        fullPath += '/';\n    }\n    fullPath += userPath;\n    \n    // Reject dangerous sequences\n    if (fullPath.find("..") != std::string::npos) {\n        return "";\n    }\n    \n    // Use realpath for canonical resolution - prevents symlink attacks\n    char resolvedPath[PATH_MAX];\n    if (realpath(fullPath.c_str(), resolvedPath) == nullptr) {\n        return "";\n    }\n    \n    std::string canonical(resolvedPath);\n    \n    // Verify resolved path is still within base directory\n    char resolvedBase[PATH_MAX];\n    if (realpath(basePath.c_str(), resolvedBase) == nullptr) {\n        return "";\n    }\n    \n    std::string canonicalBase(resolvedBase);\n    if (canonicalBase.back() != '/') {\n        canonicalBase += '/';\n    }\n    \n    if (canonical.find(canonicalBase) != 0) {\n        return "";\n    }\n    \n    return canonical;\n}\n\n// Verify file is regular file, not symlink or directory\nbool isRegularFile(const std::string& path) {\n    struct stat statbuf;\n    // Use lstat to detect symlinks\n    if (lstat(path.c_str(), &statbuf) != 0) {\n        return false;\n    }\n    return S_ISREG(statbuf.st_mode);\n}\n\n// Read file securely with bounds checking\nstd::vector<unsigned char> readFileSecurely(const std::string& filePath) {\n    std::vector<unsigned char> buffer;\n    \n    // Verify it's a regular file
    if (!isRegularFile(filePath)) {
        throw std::runtime_error("Not a regular file or is a symlink");
    }
    
    // Open file for reading
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open input file");
    }
    
    // Get file size
    std::streamsize size = file.tellg();
    if (size < 0) {
        throw std::runtime_error("Failed to determine file size");
    }
    
    // Validate file size to prevent excessive memory allocation
    const std::streamsize MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB limit
    if (size > MAX_FILE_SIZE) {
        throw std::runtime_error("File too large");
    }
    
    file.seekg(0, std::ios::beg);
    
    // Allocate buffer with bounds checking
    try {
        buffer.resize(static_cast<size_t>(size));
    } catch (const std::bad_alloc&) {
        throw std::runtime_error("Memory allocation failed");
    }
    
    // Read file content
    if (size > 0 && !file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file content");
    }
    
    file.close();
    return buffer;
}

// Write file atomically to prevent TOCTOU vulnerabilities
void writeFileAtomically(const std::string& destPath, const std::vector<unsigned char>& data) {
    // Create temporary file in same directory as destination for atomic rename
    std::string tempPath = destPath + ".tmp.XXXXXX";
    std::vector<char> tempTemplate(tempPath.begin(), tempPath.end());
    tempTemplate.push_back('\\0');
    
    // Create temporary file with secure permissions (0600)
    int fd = mkstemp(tempTemplate.data());
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    
    // Set secure permissions explicitly
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
        close(fd);
        unlink(tempTemplate.data());
        throw std::runtime_error("Failed to set file permissions");
    }
    
    std::string actualTempPath(tempTemplate.data());
    
    try {
        // Write data to temporary file
        size_t totalWritten = 0;
        while (totalWritten < data.size()) {
            ssize_t written = write(fd, data.data() + totalWritten, data.size() - totalWritten);
            if (written < 0) {
                if (errno == EINTR) continue; // Retry on interrupt
                close(fd);
                unlink(actualTempPath.c_str());
                throw std::runtime_error("Failed to write to temporary file");
            }
            totalWritten += static_cast<size_t>(written);
        }
        
        // Flush to disk to ensure data persistence
        if (fsync(fd) != 0) {
            close(fd);
            unlink(actualTempPath.c_str());
            throw std::runtime_error("Failed to sync file to disk");
        }
        
        close(fd);
        
        // Atomically rename to final destination
        if (rename(actualTempPath.c_str(), destPath.c_str()) != 0) {
            unlink(actualTempPath.c_str());
            throw std::runtime_error("Failed to rename temporary file");
        }
        
    } catch (...) {
        close(fd);
        unlink(actualTempPath.c_str());
        throw;
    }
}

// Main processing function
bool processFile(const std::string& baseDir, const std::string& inputFile, 
                 const std::string& tempDir, const std::string& outputFile) {
    try {
        // Validate and normalize input path
        std::string validatedInput = normalizePath(baseDir, inputFile);
        if (validatedInput.empty()) {
            std::cerr << "Error: Invalid input path or path escapes base directory" << std::endl;
            return false;
        }
        
        // Validate and normalize output path
        std::string validatedOutput = normalizePath(tempDir, outputFile);
        if (validatedOutput.empty()) {
            std::cerr << "Error: Invalid output path or path escapes temp directory" << std::endl;
            return false;
        }
        
        // Read file securely
        std::vector<unsigned char> fileData = readFileSecurely(validatedInput);
        
        // Write file atomically to prevent TOCTOU
        writeFileAtomically(validatedOutput, fileData);
        
        std::cout << "Successfully copied file from " << validatedInput 
                  << " to " << validatedOutput << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        // Catch and handle all exceptions with meaningful error messages
        std::cerr << "Error processing file: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return false;
    }
}

int main() {
    // Test cases with proper error handling
    std::cout << "=== Test Case 1: Valid file copy ===" << std::endl;
    processFile("/tmp/test_source", "valid.txt", "/tmp/test_dest", "output1.txt");
    
    std::cout << "\\n=== Test Case 2: Path traversal attempt ===" << std::endl;
    processFile("/tmp/test_source", "../etc/passwd", "/tmp/test_dest", "output2.txt");
    
    std::cout << "\\n=== Test Case 3: Absolute path attempt ===" << std::endl;
    processFile("/tmp/test_source", "/etc/shadow", "/tmp/test_dest", "output3.txt");
    
    std::cout << "\\n=== Test Case 4: Nonexistent file ===" << std::endl;
    processFile("/tmp/test_source", "nonexistent.txt", "/tmp/test_dest", "output4.txt");
    
    std::cout << "\\n=== Test Case 5: Empty filename ===" << std::endl;
    processFile("/tmp/test_source", "", "/tmp/test_dest", "output5.txt");
    
    return 0;
}
