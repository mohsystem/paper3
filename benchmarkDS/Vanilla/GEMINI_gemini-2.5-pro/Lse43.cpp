#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

/**
 * Reads the content of a file from the /safe/ directory.
 *
 * @param fileName The name of the file to read (not the full path).
 * @return The content of the file as a std::string, or an error message if it fails.
 */
std::string readFileFromSafeDir(const std::string& fileName) {
    // Basic check to prevent path traversal.
    if (fileName.find('/') != std::string::npos || fileName.find("..") != std::string::npos) {
        return "[ERROR] Invalid filename.";
    }

    std::string filePath = "/safe/" + fileName;
    std::ifstream fileStream(filePath);

    if (!fileStream.is_open()) {
        return "[ERROR] Could not open file: " + filePath;
    }

    // Check if it's a directory (one way to check is to try reading from it)
    if (fileStream.peek() == EOF && !fileStream.eof()) {
        // This can indicate a directory or an empty file. We will refine the check.
        // A more robust check might involve stat(), but for this example, we proceed.
        // If reading fails right away it might be a directory or permissions issue.
        fileStream.close(); // Close before returning.
        // A simple check if the path is a directory (platform-dependent without C++17)
        // Let's assume an inability to read implies an issue.
        // A better check would use std::filesystem::is_directory from C++17
        return "[ERROR] Path is a directory or file is unreadable: " + filePath;
    }
    
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();
    
    return buffer.str();
}

int main() {
    // To run these tests, you must first set up the directory and files.
    // On a Unix-like system, you can use these commands:
    // sudo mkdir /safe
    // sudo chown $(whoami) /safe
    // echo "This is a test file." > /safe/existing.txt
    // touch /safe/empty.txt
    // mkdir /safe/test_dir
    //
    // Compile with: g++ -std=c++11 -o your_program_name your_source_file.cpp

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    
    // Test Case 1: Read an existing file with content.
    std::cout << "Test Case 1: Reading 'existing.txt'" << std::endl;
    std::string content1 = readFileFromSafeDir("existing.txt");
    std::cout << "Content: \"" << content1 << "\"\n" << std::endl;

    // Test Case 2: Read a non-existent file.
    std::cout << "Test Case 2: Reading 'non_existent.txt'" << std::endl;
    std::string content2 = readFileFromSafeDir("non_existent.txt");
    std::cout << "Content: \"" << content2 << "\"\n" << std::endl;

    // Test Case 3: Read an empty file.
    std::cout << "Test Case 3: Reading 'empty.txt'" << std::endl;
    std::string content3 = readFileFromSafeDir("empty.txt");
    std::cout << "Content: \"" << content3 << "\"\n" << std::endl;
    
    // Test Case 4: Attempt to read a directory.
    std::cout << "Test Case 4: Reading directory 'test_dir'" << std::endl;
    std::string content4 = readFileFromSafeDir("test_dir");
    std::cout << "Content: \"" << content4 << "\"\n" << std::endl;

    // Test Case 5: Attempt to use an invalid filename (path traversal).
    std::cout << "Test Case 5: Reading invalid filename '../etc/passwd'" << std::endl;
    std::string content5 = readFileFromSafeDir("../etc/passwd");
    std::cout << "Content: \"" << content5 << "\"\n" << std::endl;

    return 0;
}