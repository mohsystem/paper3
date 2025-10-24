
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192
#define TEMP_PREFIX "secure_temp_"

std::string copyFileToTemp(const std::string& sourceFilePath) {
    // Validate input
    if (sourceFilePath.empty()) {
        throw std::invalid_argument("Source file path cannot be empty");
    }
    
    // Check if file exists and is readable
    struct stat fileStat;
    if (stat(sourceFilePath.c_str(), &fileStat) != 0) {
        throw std::runtime_error("Source file does not exist: " + sourceFilePath);
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        throw std::invalid_argument("Source path is not a regular file: " + sourceFilePath);
    }
    
    if (access(sourceFilePath.c_str(), R_OK) != 0) {
        throw std::runtime_error("Source file is not readable: " + sourceFilePath);
    }
    
    // Create temporary file
    char tempTemplate[] = "/tmp/secure_temp_XXXXXX";
    int tempFd = mkstemp(tempTemplate);
    
    if (tempFd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    
    std::string tempPath(tempTemplate);
    
    try {
        // Open source file
        std::ifstream sourceFile(sourceFilePath, std::ios::binary);
        if (!sourceFile) {
            close(tempFd);
            unlink(tempPath.c_str());
            throw std::runtime_error("Failed to open source file: " + sourceFilePath);
        }
        
        // Open temporary file for writing
        std::ofstream tempFile;
        tempFile.open(tempPath, std::ios::binary);
        if (!tempFile) {
            sourceFile.close();
            close(tempFd);
            unlink(tempPath.c_str());
            throw std::runtime_error("Failed to open temporary file for writing");
        }
        
        // Copy content
        char buffer[BUFFER_SIZE];
        while (sourceFile.read(buffer, BUFFER_SIZE) || sourceFile.gcount() > 0) {
            tempFile.write(buffer, sourceFile.gcount());
            if (!tempFile) {
                sourceFile.close();
                tempFile.close();
                close(tempFd);
                unlink(tempPath.c_str());
                throw std::runtime_error("Error writing to temporary file");
            }
        }
        
        sourceFile.close();
        tempFile.close();
        close(tempFd);
        
        return tempPath;
        
    } catch (...) {
        close(tempFd);
        unlink(tempPath.c_str());
        throw;
    }
}

int main() {
    // Test case 1: Valid file
    std::cout << "Test 1: Valid file" << std::endl;
    try {
        // Create a temporary test file
        char testTemplate[] = "/tmp/test1_XXXXXX";
        int testFd = mkstemp(testTemplate);
        if (testFd != -1) {
            write(testFd, "Test content 1", 14);
            close(testFd);
            
            std::string result = copyFileToTemp(testTemplate);
            std::cout << "Success: Copied to " << result << std::endl;
            unlink(testTemplate);
            unlink(result.c_str());
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Non-existent file
    std::cout << "\\nTest 2: Non-existent file" << std::endl;
    try {
        copyFileToTemp("/non/existent/file.txt");
    } catch (const std::exception& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }
    
    // Test case 3: Empty string
    std::cout << "\\nTest 3: Empty string" << std::endl;
    try {
        copyFileToTemp("");
    } catch (const std::exception& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }
    
    // Test case 4: Directory instead of file
    std::cout << "\\nTest 4: Directory instead of file" << std::endl;
    try {
        copyFileToTemp("/tmp");
    } catch (const std::exception& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }
    
    // Test case 5: Large file
    std::cout << "\\nTest 5: Large file" << std::endl;
    try {
        char testTemplate5[] = "/tmp/test5_XXXXXX";
        int testFd5 = mkstemp(testTemplate5);
        if (testFd5 != -1) {
            char largeBuffer[100000];
            for (int i = 0; i < 100000; i++) {
                largeBuffer[i] = (char)(i % 256);
            }
            write(testFd5, largeBuffer, 100000);
            close(testFd5);
            
            std::string result = copyFileToTemp(testTemplate5);
            std::cout << "Success: Large file copied to " << result << std::endl;
            unlink(testTemplate5);
            unlink(result.c_str());
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
