
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string generateRandomString(int length) {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);
    
    std::string result;
    for (int i = 0; i < length; i++) {
        result += characters[distribution(generator)];
    }
    return result;
}

std::string convertToUnicode(const std::string& input) {
    std::stringstream unicode;
    for (char c : input) {
        unicode << "\\\\u" << std::setfill('0') << std::setw(4) << std::hex << (int)(unsigned char)c;
    }
    return unicode.str();
}

std::string createTempFileWithUnicode(int stringLength) {
    // Generate random string
    std::string randomString = generateRandomString(stringLength);
    
    // Convert to Unicode
    std::string unicodeString = convertToUnicode(randomString);
    
    // Create temporary file path
    std::string tempPath;
#ifdef _WIN32
    char tempDir[MAX_PATH];
    char tempFile[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);
    GetTempFileNameA(tempDir, "tmp_", 0, tempFile);
    tempPath = tempFile;
#else
    char tempTemplate[] = "/tmp/temp_XXXXXX.txt";
    int fd = mkstemps(tempTemplate, 4);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    close(fd);
    tempPath = tempTemplate;
#endif
    
    // Write to file with UTF-8 encoding
    std::ofstream file(tempPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open temporary file");
    }
    
    file << unicodeString;
    file.close();
    
    return tempPath;
}

int main() {
    try {
        // Test case 1
        std::string path1 = createTempFileWithUnicode(10);
        std::cout << "Test 1 - Temp file created: " << path1 << std::endl;
        
        // Test case 2
        std::string path2 = createTempFileWithUnicode(20);
        std::cout << "Test 2 - Temp file created: " << path2 << std::endl;
        
        // Test case 3
        std::string path3 = createTempFileWithUnicode(5);
        std::cout << "Test 3 - Temp file created: " << path3 << std::endl;
        
        // Test case 4
        std::string path4 = createTempFileWithUnicode(15);
        std::cout << "Test 4 - Temp file created: " << path4 << std::endl;
        
        // Test case 5
        std::string path5 = createTempFileWithUnicode(25);
        std::cout << "Test 5 - Temp file created: " << path5 << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
