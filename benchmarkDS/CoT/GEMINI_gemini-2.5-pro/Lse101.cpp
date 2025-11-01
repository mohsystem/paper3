#include <iostream>
#include <fstream>
#include <string>

/**
 * @brief Opens a file with the given name and writes the provided content to it.
 * The file is created if it does not exist, and overwritten if it does.
 *
 * @param fileName The name of the file to write to.
 * @param content The string content to write into the file.
 * @return true if the write was successful, false otherwise.
 */
bool writeToFile(const std::string& fileName, const std::string& content) {
    // std::ofstream handles file opening.
    // Its destructor will automatically close the file when the object goes out of scope (RAII).
    // This is a security best practice that prevents resource leaks.
    std::ofstream outFile(fileName);

    // Check if the file was successfully opened.
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << fileName << "' for writing." << std::endl;
        return false;
    }

    outFile << content;

    // Check for write errors
    if (outFile.fail()) {
        std::cerr << "Error: Failed to write to file '" << fileName << "'." << std::endl;
        return false;
    }
    
    // The file is automatically closed here when 'outFile' is destroyed.
    return true;
}

int main() {
    // --- Test Cases ---

    // Test Case 1: The original prompt
    std::cout << "Test Case 1: Writing to 'important_config'" << std::endl;
    if (writeToFile("important_config", "important_config")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    // Test Case 2: Writing to a different file with different content
    std::cout << "Test Case 2: Writing to 'test_config_1.txt'" << std::endl;
    if (writeToFile("test_config_1.txt", "test data 1")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    // Test Case 3: Another file type
    std::cout << "Test Case 3: Writing to 'another.cfg'" << std::endl;
    if (writeToFile("another.cfg", "some other config")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    // Test Case 4: Writing an empty string to a file
    std::cout << "Test Case 4: Writing to 'empty_file.txt'" << std::endl;
    if (writeToFile("empty_file.txt", "")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    // Test Case 5: Writing content with special characters
    std::cout << "Test Case 5: Writing to 'special_chars_config.log'" << std::endl;
    if (writeToFile("special_chars_config.log", "log data with !@#$%^&*()")) {
        std::cout << "Success." << std::endl;
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    return 0;
}