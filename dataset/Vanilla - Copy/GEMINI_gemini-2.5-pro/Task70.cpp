#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio> // For std::remove

/**
 * Reads the entire content of a file into a string.
 *
 * @param filePath The path to the file.
 * @return The content of the file as a string, or an error message if the file cannot be read.
 */
std::string readFileContent(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        return "Error: Could not open file '" + filePath + "'.";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();
    return buffer.str();
}

// Helper function to create a test file
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }
}

int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        std::string filePath = argv[1];
        std::cout << "Reading content from command line argument: " << filePath << std::endl;
        std::cout << "--- File Content ---" << std::endl;
        std::cout << readFileContent(filePath) << std::endl;
        std::cout << "--- End of Content ---" << std::endl;
        return 0; // Exit after processing command line arg
    }
    
    // Run built-in test cases if no command-line argument is provided
    std::cout << "No command line argument provided. Running built-in test cases..." << std::endl;
    std::cout << "=========================================================" << std::endl;

    // Test Case 1: Simple file
    std::cout << "Test Case 1: Reading a simple file (test1.txt)" << std::endl;
    createTestFile("test1.txt", "Hello World from C++!");
    std::cout << "Content: " << readFileContent("test1.txt") << std::endl;
    std::remove("test1.txt");
    std::cout << "---------------------------------------------------------" << std::endl;
    
    // Test Case 2: File with multiple lines
    std::cout << "Test Case 2: Reading a multi-line file (test2.txt)" << std::endl;
    createTestFile("test2.txt", "Line 1\nLine 2\nLine 3");
    std::cout << "Content:\n" << readFileContent("test2.txt") << std::endl;
    std::remove("test2.txt");
    std::cout << "---------------------------------------------------------" << std::endl;

    // Test Case 3: Empty file
    std::cout << "Test Case 3: Reading an empty file (test3.txt)" << std::endl;
    createTestFile("test3.txt", "");
    std::cout << "Content: '" << readFileContent("test3.txt") << "'" << std::endl;
    std::remove("test3.txt");
    std::cout << "---------------------------------------------------------" << std::endl;

    // Test Case 4: Non-existent file
    std::cout << "Test Case 4: Attempting to read a non-existent file" << std::endl;
    std::cout << "Content: " << readFileContent("nonexistentfile.txt") << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    
    // Test Case 5: File with special characters
    std::cout << "Test Case 5: Reading a file with special characters (test5.txt)" << std::endl;
    createTestFile("test5.txt", "UTF-8 test: áéíóú ñ €");
    std::cout << "Content: " << readFileContent("test5.txt") << std::endl;
    std::remove("test5.txt");
    std::cout << "---------------------------------------------------------" << std::endl;

    std::cout << "=========================================================" << std::endl;
    
    return 0;
}