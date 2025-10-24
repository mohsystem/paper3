#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio> // For std::remove

// In C++17 and later, you can use std::filesystem for more robust path handling.
// For this example, we'll stick to more portable C++11 features.
#include <sys/stat.h> // For stat to check if it's a file

/**
 * Checks if a given path corresponds to a regular file.
 *
 * @param name The path to check.
 * @return True if it's a regular file, false otherwise.
 */
bool isRegularFile(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

/**
 * Reads the content of a file after validating the filename.
 *
 * @param filename The name of the file to read.
 * @return The content of the file as a std::string. Returns an empty string on error.
 */
std::string readFileContent(const std::string& filename) {
    if (filename.empty()) {
        std::cerr << "Error: Filename cannot be empty." << std::endl;
        return "";
    }

    // Security: Prevent path traversal attacks.
    if (filename.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid filename. Path traversal is not allowed." << std::endl;
        return "";
    }
    
    // Basic validation
    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Could not open file (check existence and permissions): " << filename << std::endl;
        return "";
    }
    
    if (!isRegularFile(filename)) {
        std::cerr << "Error: Path is not a regular file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();

    return buffer.str();
}

// Helper to create a test file
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create test file: " << filename << std::endl;
    }
}

// Helper to run test cases
void runTests() {
    std::cout << "--- Running Tests ---" << std::endl;

    // Test Case 1: Valid file with content
    std::cout << "\n--- Test Case 1: Read a valid file ---" << std::endl;
    std::string testFile1 = "test1.txt";
    std::string content1 = "Hello World!";
    createTestFile(testFile1, content1);
    std::string result1 = readFileContent(testFile1);
    std::cout << "Expected: " << content1 << std::endl;
    std::cout << "Got: " << result1 << std::endl;
    std::cout << "Test " << (content1 == result1 ? "PASSED" : "FAILED") << std::endl;

    // Test Case 2: Empty file
    std::cout << "\n--- Test Case 2: Read an empty file ---" << std::endl;
    std::string testFile2 = "test2.txt";
    createTestFile(testFile2, "");
    std::string result2 = readFileContent(testFile2);
    std::cout << "Expected: " << std::endl;
    std::cout << "Got: " << result2 << std::endl;
    std::cout << "Test " << ("" == result2 ? "PASSED" : "FAILED") << std::endl;

    // Test Case 3: Non-existent file
    std::cout << "\n--- Test Case 3: Read a non-existent file ---" << std::endl;
    std::string result3 = readFileContent("nonexistent.txt");
    std::cout << "Expected: (empty string with error message)" << std::endl;
    std::cout << "Got: " << result3 << std::endl;
    std::cout << "Test " << (result3.empty() ? "PASSED" : "FAILED") << std::endl;

    // Test Case 4: Path traversal attempt
    std::cout << "\n--- Test Case 4: Path traversal attempt ---" << std::endl;
    std::string result4 = readFileContent("../somefile.txt");
    std::cout << "Expected: (empty string with error message)" << std::endl;
    std::cout << "Got: " << result4 << std::endl;
    std::cout << "Test " << (result4.empty() ? "PASSED" : "FAILED") << std::endl;

    // Test Case 5: File with multi-line content
    std::cout << "\n--- Test Case 5: Read a multi-line file ---" << std::endl;
    std::string testFile5 = "test5.txt";
    std::string content5 = "Line 1\nLine 2\nLine 3";
    createTestFile(testFile5, content5);
    std::string result5 = readFileContent(testFile5);
    std::cout << "Expected:\n" << content5 << std::endl;
    std::cout << "Got:\n" << result5 << std::endl;
    std::cout << "Test " << (content5 == result5 ? "PASSED" : "FAILED") << std::endl;

    // Cleanup
    std::remove(testFile1.c_str());
    std::remove(testFile2.c_str());
    std::remove(testFile5.c_str());
    
    std::cout << "\n--- Tests Finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string filename = argv[1];
        std::cout << "Attempting to read file from command line: " << filename << std::endl;
        std::string content = readFileContent(filename);
        if (!content.empty() || readFileContent(filename) == "") { // Check if file was truly empty vs. error
            std::ifstream check(filename);
            if (check.peek() == std::ifstream::traits_type::eof() && content.empty()) {
                 std::cout << "\n--- File is empty ---" << std::endl;
            } else if (!content.empty()){
                std::cout << "\n--- File Content ---" << std::endl;
                std::cout << content << std::endl;
                std::cout << "--- End of Content ---" << std::endl;
            } else {
                 std::cout << "Failed to read file." << std::endl;
            }
        }
    } else {
        std::cout << "No filename provided via command line. Running built-in test cases." << std::endl;
        runTests();
    }
    return 0;
}