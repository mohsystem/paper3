#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype> // For isalpha
#include <cstdio> // For remove

// Function to check if a path is potentially unsafe.
// This is a basic check and might not be exhaustive.
// It prevents directory traversal and absolute paths.
bool isPathSafe(const std::string& path) {
    if (path.find("..") != std::string::npos) {
        std::cerr << "Error: Path traversal detected." << std::endl;
        return false;
    }
    if (!path.empty() && path[0] == '/') {
        std::cerr << "Error: Absolute paths are not allowed." << std::endl;
        return false;
    }
    // A simple check for Windows absolute paths like "C:\"
    if (path.length() > 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') {
        std::cerr << "Error: Absolute paths are not allowed." << std::endl;
        return false;
    }
    return true;
}

// Function to print file contents.
void printFileContents(const std::string& filename) {
    if (!isPathSafe(filename)) {
        return;
    }

    // std::ifstream will fail to open directories, which helps prevent reading them.
    // This avoids a check-then-open (TOCTOU) vulnerability.
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'. It might not exist, be a directory, or you may lack permissions." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    if (file.bad()) {
        std::cerr << "Error: A non-recoverable error occurred while reading the file." << std::endl;
    }
    
    // file is closed automatically by its destructor (RAII)
}

void runTestCase(const std::string& testName, const std::string& filename, const std::string& content, bool createFile) {
    std::cout << "--- " << testName << " ---" << std::endl;
    std::cout << "Attempting to read: " << filename << std::endl;

    if (createFile) {
        std::ofstream testFile(filename);
        if (testFile.is_open()) {
            testFile << content;
            testFile.close();
        } else {
            std::cerr << "Failed to create test file: " << filename << std::endl;
            return;
        }
    }

    printFileContents(filename);

    if (createFile) {
        remove(filename.c_str());
    }

    std::cout << "--------------------" << std::endl << std::endl;
}

int main() {
    // Test Case 1: Standard file with multiple lines
    runTestCase("Test Case 1: Standard File", "test1.txt", "Hello, World!\nThis is a test file.", true);

    // Test Case 2: Empty file
    runTestCase("Test Case 2: Empty File", "test2.txt", "", true);

    // Test Case 3: File with a single line
    runTestCase("Test Case 3: Single Line File", "test3.txt", "One line only.", true);

    // Test Case 4: Non-existent file
    runTestCase("Test Case 4: Non-Existent File", "nonexistent.txt", "", false);

    // Test Case 5: Unsafe path (directory traversal)
    runTestCase("Test Case 5: Unsafe Path", "../unsafe.txt", "This should not be accessed.", false);

    return 0;
}