
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string readFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "Error reading file: Unable to open file";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

int main() {
    // Test case 1: Create and read a simple text file
    {
        std::ofstream outFile("test1.txt");
        outFile << "Hello, World!" << std::endl;
        outFile << "This is test file 1." << std::endl;
        outFile.close();
        std::cout << "Test Case 1:" << std::endl;
        std::cout << readFileContents("test1.txt") << std::endl;
    }
    
    // Test case 2: Create and read a file with multiple lines
    {
        std::ofstream outFile("test2.txt");
        outFile << "Line 1" << std::endl;
        outFile << "Line 2" << std::endl;
        outFile << "Line 3" << std::endl;
        outFile.close();
        std::cout << "Test Case 2:" << std::endl;
        std::cout << readFileContents("test2.txt") << std::endl;
    }
    
    // Test case 3: Empty file
    {
        std::ofstream outFile("test3.txt");
        outFile.close();
        std::cout << "Test Case 3 (Empty file):" << std::endl;
        std::cout << readFileContents("test3.txt") << std::endl;
    }
    
    // Test case 4: File with special characters
    {
        std::ofstream outFile("test4.txt");
        outFile << "Special chars: @#$%^&*()" << std::endl;
        outFile << "Numbers: 123456789" << std::endl;
        outFile.close();
        std::cout << "Test Case 4:" << std::endl;
        std::cout << readFileContents("test4.txt") << std::endl;
    }
    
    // Test case 5: Non-existent file
    std::cout << "Test Case 5 (Non-existent file):" << std::endl;
    std::cout << readFileContents("nonexistent.txt") << std::endl;
    
    return 0;
}
