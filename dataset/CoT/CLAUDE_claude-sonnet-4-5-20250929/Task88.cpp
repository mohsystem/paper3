
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cstdlib>
#include <ctime>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
const int RANDOM_STRING_LENGTH = 20;

std::string generateSecureRandomString(int length) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
    
    std::string randomString;
    randomString.reserve(length);
    
    for (int i = 0; i < length; i++) {
        randomString += CHARACTERS[distribution(generator)];
    }
    
    return randomString;
}

std::string createTempFileWithRandomString() {
    // Generate secure random string
    std::string randomString = generateSecureRandomString(RANDOM_STRING_LENGTH);
    
    // Create temporary file path
    std::string tempPath;
    
#ifdef _WIN32
    char tempDir[MAX_PATH];
    char tempFileName[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);
    GetTempFileNameA(tempDir, "secure_temp_", 0, tempFileName);
    tempPath = tempFileName;
#else
    char tempTemplate[] = "/tmp/secure_temp_XXXXXX";
    int fd = mkstemp(tempTemplate);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    tempPath = tempTemplate;
    
    // Set restrictive permissions (0600)
    fchmod(fd, S_IRUSR | S_IWUSR);
    close(fd);
#endif
    
    // Write UTF-8 encoded string to file
    std::ofstream outFile(tempPath, std::ios::out | std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open temporary file");
    }
    
#ifdef _WIN32
    // Set file permissions on Windows
    DWORD attrs = GetFileAttributesA(tempPath.c_str());
    SetFileAttributesA(tempPath.c_str(), attrs | FILE_ATTRIBUTE_TEMPORARY);
#endif
    
    outFile.write(randomString.c_str(), randomString.length());
    outFile.close();
    
    return tempPath;
}

int main() {
    std::cout << "Testing temporary file creation with random Unicode strings:\\n\\n";
    
    for (int i = 1; i <= 5; i++) {
        try {
            std::string filePath = createTempFileWithRandomString();
            std::cout << "Test " << i << ": " << filePath << std::endl;
            
            // Verify file content
            std::ifstream inFile(filePath);
            std::string content((std::istreambuf_iterator<char>(inFile)),
                               std::istreambuf_iterator<char>());
            inFile.close();
            
            std::cout << "  Content: " << content << std::endl;
            std::cout << "  Length: " << content.length() << " characters\\n\\n";
        } catch (const std::exception& e) {
            std::cerr << "Test " << i << " failed: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
