
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <limits>

#define MAX_FILE_SIZE (10 * 1024 * 1024)  // 10MB limit

std::string readFileContent(const char* filePath) {
    if (filePath == nullptr || strlen(filePath) == 0) {
        return "Error: File path cannot be null or empty";
    }
    
    // Check if file exists and get file size
    struct stat fileStat;
    if (stat(filePath, &fileStat) != 0) {
        return "Error: File does not exist or cannot be accessed: " + std::string(filePath);
    }
    
    // Check if it's a regular file
    if (!S_ISREG(fileStat.st_mode)) {
        return "Error: Path is not a regular file: " + std::string(filePath);
    }
    
    // Check file size to prevent memory exhaustion
    if (fileStat.st_size > MAX_FILE_SIZE) {
        return "Error: File size exceeds maximum allowed size (10MB)";
    }
    
    // Open file for reading
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "Error: Unable to open file: " + std::string(filePath);
    }
    
    try {
        // Read file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    } catch (const std::exception& e) {
        file.close();
        return "Error: Unable to read file - " + std::string(e.what());
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
        std::cout << "\\nRunning test cases...\\n" << std::endl;
        
        // Test case 1: Valid file (create a test file)
        std::cout << "Test 1 - Valid file:" << std::endl;
        const char* testFile1 = "test1_temp.txt";
        std::ofstream out1(testFile1);
        out1 << "Hello, World!";
        out1.close();
        std::cout << readFileContent(testFile1) << std::endl;
        remove(testFile1);
        
        // Test case 2: Non-existent file
        std::cout << "\\nTest 2 - Non-existent file:" << std::endl;
        std::cout << readFileContent("/nonexistent/file.txt") << std::endl;
        
        // Test case 3: Null path
        std::cout << "\\nTest 3 - Null path:" << std::endl;
        std::cout << readFileContent(nullptr) << std::endl;
        
        // Test case 4: Empty path
        std::cout << "\\nTest 4 - Empty path:" << std::endl;
        std::cout << readFileContent("") << std::endl;
        
        // Test case 5: File with special characters
        std::cout << "\\nTest 5 - File with special characters:" << std::endl;
        const char* testFile5 = "test5_temp.txt";
        std::ofstream out5(testFile5);
        out5 << "Content with special chars: @#$%^&*()";
        out5.close();
        std::cout << readFileContent(testFile5) << std::endl;
        remove(testFile5);
        
    } else {
        const char* filePath = argv[1];
        std::string content = readFileContent(filePath);
        std::cout << content << std::endl;
    }
    
    return 0;
}
