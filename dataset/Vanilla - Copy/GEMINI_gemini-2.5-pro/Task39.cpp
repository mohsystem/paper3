#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib> // For std::system
#include <cstdio>  // For std::remove

/**
 * @brief Displays the content of a file using a system command.
 * This function is cross-platform, using 'type' on Windows and 'cat' on others.
 * @param filename The name of the file to display.
 */
void displayFileContent(const std::string& filename) {
    std::string command;
    // Use preprocessor directives to determine the OS at compile time
#ifdef _WIN32
    // For Windows, use the 'type' command. Quotes handle spaces in filenames.
    command = "type \"" + filename + "\"";
#else
    // For Unix-like systems, use the 'cat' command.
    command = "cat \"" + filename + "\"";
#endif
    
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Command failed or returned a non-zero exit code for file: " << filename << std::endl;
    }
}

int main() {
    std::vector<std::string> testFiles;
    for (int i = 1; i <= 5; ++i) {
        testFiles.push_back("cpp_test_file_" + std::to_string(i) + ".txt");
    }

    std::cout << "--- Creating test files ---" << std::endl;
    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::ofstream outFile(testFiles[i]);
        if (outFile.is_open()) {
            outFile << "Hello from C++ test file " << (i + 1) << ".\n";
            outFile << "This is the content of " << testFiles[i] << ".\n";
            outFile.close();
            std::cout << "Created: " << testFiles[i] << std::endl;
        } else {
            std::cerr << "Error creating file: " << testFiles[i] << std::endl;
            return 1;
        }
    }

    std::cout << "\n--- Running C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << ": Displaying " << testFiles[i] << " ---" << std::endl;
        displayFileContent(testFiles[i]);
        std::cout << "--- End of " << testFiles[i] << " ---" << std::endl;
    }

    std::cout << "\n--- Cleaning up test files ---" << std::endl;
    for (const auto& filename : testFiles) {
        if (std::remove(filename.c_str()) != 0) {
             std::cerr << "Error deleting file: " << filename << std::endl;
        } else {
            std::cout << "Deleted: " << filename << std::endl;
        }
    }

    return 0;
}