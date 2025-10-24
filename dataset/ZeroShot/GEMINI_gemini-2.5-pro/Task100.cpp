#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

/**
 * Searches a file for lines matching a given regular expression.
 *
 * @param regexPattern The regular expression pattern to search for.
 * @param filePath     The path to the file to be searched.
 */
void searchFileWithRegex(const std::string& regexPattern, const std::string& filePath) {
    std::regex pattern;
    try {
        // Security: This can throw std::regex_error on invalid patterns.
        pattern = std::regex(regexPattern);
    } catch (const std::regex_error& e) {
        std::cerr << "Error: Invalid regular expression pattern: " << e.what() << std::endl;
        return;
    }

    // Security: RAII ensures the file is closed when 'file' goes out of scope.
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        // Security: std::regex_search is generally safe, but complex patterns
        // on long lines can be slow (potential ReDoS).
        if (std::regex_search(line, pattern)) {
            std::cout << filePath << ":" << lineNumber << ":" << line << std::endl;
        }
    }
}

void runTests() {
    const std::string testFileName = "test_file.txt";
    // Create a test file
    std::ofstream outFile(testFileName);
    if (!outFile) {
        std::cerr << "Failed to create test file." << std::endl;
        return;
    }
    outFile << "Hello world, this is a test file.\n";
    outFile << "The year is 2024.\n";
    outFile << "Let's test some patterns.\n";
    outFile << "Another line for testing.\n";
    outFile << "bat, bit, but, b@t.\n";
    outFile << "Goodbye!\n";
    outFile.close();

    std::cout << "\n--- Test Case 1: Simple Match ('test') ---" << std::endl;
    searchFileWithRegex("test", testFileName);

    std::cout << "\n--- Test Case 2: No Match ('nomatch') ---" << std::endl;
    searchFileWithRegex("nomatch", testFileName);

    std::cout << "\n--- Test Case 3: Regex Special Chars ('b.t') ---" << std::endl;
    searchFileWithRegex("b.t", testFileName);

    std::cout << "\n--- Test Case 4: Digit Match ('[0-9]+') ---" << std::endl;
    searchFileWithRegex("[0-9]+", testFileName);

    std::cout << "\n--- Test Case 5: Non-existent File ---" << std::endl;
    searchFileWithRegex("hello", "nonexistent.txt");
    
    // Clean up the test file
    remove(testFileName.c_str());
}

int main(int argc, char* argv[]) {
    // Security: Check for the correct number of command-line arguments.
    if (argc == 3) {
        searchFileWithRegex(argv[1], argv[2]);
    } else {
        std::cout << "Usage: " << argv[0] << " <regex_pattern> <file_path>" << std::endl;
        std::cout << "Running test cases instead..." << std::endl;
        runTests();
    }
    return 0;
}