#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/**
 * Opens a file and writes the given content to it, overwriting existing content.
 * @param fileName The name of the file to write to.
 * @param content The string content to write to the file.
 * @return true on success, false on failure.
 */
bool writeToFile(const std::string& fileName, const std::string& content) {
    // std::ofstream opens a file for writing.
    // The default mode creates the file if it doesn't exist and truncates it if it does.
    std::ofstream outFile(fileName);

    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << fileName << std::endl;
        return false;
    }

    outFile << content;
    outFile.close(); // Good practice, though destructor would handle it upon scope exit.
    return true;
}

/**
 * Helper function to read a file and verify its content.
 * @param fileName The name of the file to read.
 * @param expectedContent The content expected to be in the file.
 */
void verifyFileContent(const std::string& fileName, const std::string& expectedContent) {
    std::ifstream inFile(fileName);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open file for verification: " << fileName << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string fileContent = buffer.str();
    inFile.close();

    if (fileContent == expectedContent) {
        std::cout << "Verification SUCCESS for " << fileName << std::endl;
    } else {
        std::cout << "Verification FAILED for " << fileName << std::endl;
        std::cout << "Expected: '" << expectedContent << "'" << std::endl;
        std::cout << "Got: '" << fileContent << "'" << std::endl;
    }
}


int main() {
    // Test Case 1
    std::cout << "--- Test Case 1 ---" << std::endl;
    std::string file1 = "important_config";
    std::string content1 = "important_config";
    if (writeToFile(file1, content1)) {
        std::cout << "Successfully wrote to " << file1 << std::endl;
        verifyFileContent(file1, content1);
    }
    std::cout << std::endl;

    // Test Case 2
    std::cout << "--- Test Case 2 ---" << std::endl;
    std::string file2 = "test1.txt";
    std::string content2 = "Hello World";
    if (writeToFile(file2, content2)) {
        std::cout << "Successfully wrote to " << file2 << std::endl;
        verifyFileContent(file2, content2);
    }
    std::cout << std::endl;

    // Test Case 3
    std::cout << "--- Test Case 3 ---" << std::endl;
    std::string file3 = "data.log";
    std::string content3 = "Log entry: 2023-10-27";
    if (writeToFile(file3, content3)) {
        std::cout << "Successfully wrote to " << file3 << std::endl;
        verifyFileContent(file3, content3);
    }
    std::cout << std::endl;

    // Test Case 4
    std::cout << "--- Test Case 4 ---" << std::endl;
    std::string file4 = "empty_file.txt";
    std::string content4 = "";
    if (writeToFile(file4, content4)) {
        std::cout << "Successfully wrote to " << file4 << std::endl;
        verifyFileContent(file4, content4);
    }
    std::cout << std::endl;

    // Test Case 5
    std::cout << "--- Test Case 5 ---" << std::endl;
    std::string file5 = "another_config.ini";
    std::string content5 = "[settings]\nuser=admin";
    if (writeToFile(file5, content5)) {
        std::cout << "Successfully wrote to " << file5 << std::endl;
        verifyFileContent(file5, content5);
    }
    std::cout << std::endl;

    return 0;
}