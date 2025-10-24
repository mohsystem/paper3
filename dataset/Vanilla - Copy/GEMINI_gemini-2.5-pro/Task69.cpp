#include <iostream>
#include <string>
#include <vector>
#include <cstdio>   // For remove()
#include <fstream>  // For creating files for testing

/**
 * Deletes a file at the given file path.
 *
 * @param filePath The path of the file to delete.
 * @return true if the file was successfully deleted, false otherwise.
 */
bool deleteFile(const std::string& filePath) {
    // remove() returns 0 on success and a non-zero value on failure.
    if (remove(filePath.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Creates and runs 5 test cases for the deleteFile function.
 */
void runTestCases() {
    std::cout << "No command line arguments provided. Running test cases..." << std::endl;
    std::vector<std::string> testFiles = {"test1.tmp", "test2.tmp", "test3.tmp", "test4.tmp", "test5.tmp"};
    
    // Test Cases 1-5: Create and then delete a file.
    for (size_t i = 0; i < testFiles.size(); ++i) {
        const std::string& fileName = testFiles[i];
        std::cout << "\n--- Test Case " << (i + 1) << ": Delete " << fileName << " ---" << std::endl;
        
        // 1. Create a dummy file for the test.
        std::ofstream outfile(fileName);
        if (!outfile) {
            std::cerr << "  [Setup] Error: Could not create temporary file: " << fileName << std::endl;
            continue; // Skip test if setup fails
        }
        outfile.close();
        std::cout << "  [Setup] Created temporary file: " << fileName << std::endl;

        // 2. Attempt to delete the file.
        if (deleteFile(fileName)) {
            std::cout << "  [Result] SUCCESS: File deleted." << std::endl;
        } else {
            std::cout << "  [Result] FAILED: File not deleted." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, use it as the file path.
    if (argc > 1) {
        std::string filePath = argv[1];
        std::cout << "Attempting to delete file from command line: " << filePath << std::endl;
        if (deleteFile(filePath)) {
            std::cout << "File deleted successfully." << std::endl;
        } else {
            std::cerr << "Failed to delete the file. It may not exist or you may not have permission." << std::endl;
        }
    } else {
        // If no arguments, run the built-in test cases.
        runTestCases();
    }
    return 0;
}