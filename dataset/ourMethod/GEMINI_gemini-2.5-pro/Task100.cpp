#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <stdexcept>
#include <vector>
#include <filesystem> // For C++17 filesystem features
#include <cstdio> // For std::remove

/**
 * Searches a file for lines matching a regular expression.
 *
 * @param patternStr The regular expression to search for.
 * @param filePath   The path to the file to search in.
 */
void processFile(const std::string& patternStr, const std::string& filePath) {
    if (!std::filesystem::is_regular_file(filePath)) {
        std::cerr << "Error: Path provided is not a regular file: " << filePath << std::endl;
        return;
    }

    std::regex pattern;
    try {
        pattern = std::regex(patternStr);
    } catch (const std::regex_error& e) {
        std::cerr << "Error: Invalid regular expression: " << e.what() << std::endl;
        return;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (std::regex_search(line, pattern)) {
            std::cout << line << std::endl;
        }
    }
}

/**
 * Runs a series of test cases.
 */
void runTests() {
    std::cout << "Running built-in tests..." << std::endl;
    const char* testFilename = "test_regex_search_cpp.txt";
    
    // Create and write to the test file
    std::ofstream testFile(testFilename);
    if (!testFile.is_open()) {
        std::cerr << "Failed to create test file." << std::endl;
        return;
    }
    testFile << "Hello World\n";
    testFile << "This is a test file.\n";
    testFile << "The quick brown fox jumps over the lazy dog.\n";
    testFile << "Contact us at test@example.com or support@example.org.\n";
    testFile << "Phone numbers: 123-456-7890, (987)654-3210.\n";
    testFile << "Another line with numbers 12345.\n";
    testFile << "end of file.\n";
    testFile.close();

    std::cout << "\n--- Test Case 1: Find lines with 'fox' ---" << std::endl;
    processFile("fox", testFilename);

    std::cout << "\n--- Test Case 2: Find lines with any number ---" << std::endl;
    processFile("\\d+", testFilename);

    std::cout << "\n--- Test Case 3: Find lines with email addresses ---" << std::endl;
    processFile("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", testFilename);

    std::cout << "\n--- Test Case 4: Find lines starting with 'The' ---" << std::endl;
    processFile("^The", testFilename);

    std::cout << "\n--- Test Case 5: Find lines with 'nonexistentpattern' ---" << std::endl;
    processFile("nonexistentpattern", testFilename);
    std::cout << "(No output expected)" << std::endl;

    // Clean up the test file
    std::remove(testFilename);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        processFile(argv[1], argv[2]);
    } else {
        std::cout << "Usage: " << argv[0] << " \"<regex_pattern>\" \"<file_name>\"" << std::endl;
        std::cout << "No command-line arguments provided, running test cases." << std::endl;
        runTests();
    }
    return 0;
}