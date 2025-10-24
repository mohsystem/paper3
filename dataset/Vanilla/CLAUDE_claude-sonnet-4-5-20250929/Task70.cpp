
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string readFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        return "Error reading file: File not found - " + filePath;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testFiles[] = {
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "nonexistent.txt",
        "test5.txt"
    };
    
    // Create test files
    std::ofstream file1("test1.txt");
    file1 << "Hello, World!";
    file1.close();
    
    std::ofstream file2("test2.txt");
    file2 << "C++ Programming";
    file2.close();
    
    std::ofstream file3("test3.txt");
    file3 << "File I/O Operations";
    file3.close();
    
    std::ofstream file5("test5.txt");
    file5 << "Test Case 5";
    file5.close();
    
    // If command line argument provided, use it
    if (argc > 1) {
        std::string content = readFileContent(argv[1]);
        std::cout << "File Content:" << std::endl;
        std::cout << content << std::endl;
    } else {
        // Run test cases
        std::cout << "Running test cases:" << std::endl << std::endl;
        for (int i = 0; i < 5; i++) {
            std::cout << "Test Case " << (i + 1) << ": " << testFiles[i] << std::endl;
            std::string content = readFileContent(testFiles[i]);
            std::cout << "Content: " << content << std::endl;
            std::cout << "---" << std::endl;
        }
    }
    
    return 0;
}
