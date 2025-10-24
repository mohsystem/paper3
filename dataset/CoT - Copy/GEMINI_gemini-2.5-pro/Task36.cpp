#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * Reads a file specified by the file path and prints its contents to the console.
 * It handles file opening errors gracefully.
 *
 * @param filePath The path to the file to be read.
 */
void readFileAndPrint(const std::string& filePath) {
    // Security: Basic input validation.
    if (filePath.empty()) {
        std::cerr << "Error: File path cannot be empty." << std::endl;
        return;
    }

    // Security: std::ifstream's destructor will automatically close the file
    // when 'file' goes out of scope (RAII principle), preventing resource leaks.
    std::ifstream file(filePath);

    // Security: Check if the file was opened successfully. This handles
    // cases where the file doesn't exist, is a directory, or permissions are denied.
    if (!file.is_open()) {
        // Using perror equivalent for more descriptive error.
        // C++ standard library doesn't have a direct equivalent for all platforms,
        // so a generic message is used. std::system_error is an option but more complex.
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    std::string line;
    // Security: Reading line by line with std::getline is safe and avoids
    // buffer overflows that can occur with fixed-size C-style char arrays.
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    std::string testFileName = "cpp_test_file.txt";

    // Setup: Create a temporary file for testing
    {
        std::ofstream outFile(testFileName);
        if (!outFile) {
            std::cerr << "Failed to create test file." << std::endl;
            return 1;
        }
        outFile << "Hello from the C++ test file." << std::endl;
        outFile << "This is the second line." << std::endl;
        outFile << "End of file." << std::endl;
    } // outFile is closed here by its destructor

    // Test Case 1: Read a valid, existing file.
    std::cout << "\n[Test Case 1: Reading a valid file]" << std::endl;
    readFileAndPrint(testFileName);

    // Test Case 2: Attempt to read a non-existent file.
    std::cout << "\n[Test Case 2: Reading a non-existent file]" << std::endl;
    readFileAndPrint("non_existent_file.txt");

    // Test Case 3: Attempt to read with a different valid file (for variety)
    std::cout << "\n[Test Case 3: Another valid file read]" << std::endl;
    // Re-using the test file for this case to keep it self-contained
    readFileAndPrint(testFileName);

    // Test Case 4: Attempt to read with an empty path.
    std::cout << "\n[Test Case 4: Reading an empty path]" << std::endl;
    readFileAndPrint("");

    // Test Case 5: Attempt to read a directory.
    std::cout << "\n[Test Case 5: Reading a directory]" << std::endl;
    readFileAndPrint("."); // "." represents the current directory

    // Cleanup: Delete the temporary test file
    if (std::remove(testFileName.c_str()) != 0) {
        std::cerr << "Failed to delete test file." << std::endl;
    }

    return 0;
}