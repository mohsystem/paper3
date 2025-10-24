#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

/**
 * Searches for a regex pattern within a file and prints matching lines.
 * @param patternStr The regular expression pattern to search for.
 * @param fileName The path to the file to search in.
 */
void searchInFile(const std::string& patternStr, const std::string& fileName) {
    std::cout << "Searching for pattern '" << patternStr << "' in file '" << fileName << "'..." << std::endl;
    
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        std::cout << "------------------------------------" << std::endl;
        return;
    }

    try {
        std::regex pattern(patternStr);
        std::string line;
        while (std::getline(file, line)) {
            if (std::regex_search(line, pattern)) {
                std::cout << "Found match: " << line << std::endl;
            }
        }
    } catch (const std::regex_error& e) {
        std::cerr << "Regex error: " << e.what() << std::endl;
    }
    
    file.close();
    std::cout << "------------------------------------" << std::endl;
}

/**
 * Sets up test data and runs test cases.
 */
void runTestCases() {
    const std::string testFileName = "test_file_cpp.txt";
    std::ofstream testFile(testFileName);
    if (!testFile.is_open()) {
        std::cerr << "Failed to create test file." << std::endl;
        return;
    }

    testFile << "Hello world, this is a test file.\n";
    testFile << "The year is 2024.\n";
    testFile << "Contact us at test@example.com for more info.\n";
    testFile << "Or call 123-456-7890.\n";
    testFile << "This line contains no special characters or numbers\n";
    testFile << "Another email: another.email@domain.org.\n";
    testFile.close();

    // Test Case 1: Match a 4-digit number
    searchInFile("\\d{4}", testFileName);

    // Test Case 2: Match an email address
    searchInFile("\\w+@\\w+\\.\\w+", testFileName);

    // Test Case 3: Match a line starting with "Hello"
    searchInFile("^Hello", testFileName);

    // Test Case 4: Match a line ending with "numbers"
    searchInFile("numbers$", testFileName);

    // Test Case 5: A pattern that won't match anything
    searchInFile("NonExistentPatternXYZ", testFileName);

    // Cleanup the test file
    remove(testFileName.c_str());
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        // Use command-line arguments
        std::string regexPattern = argv[1];
        std::string fileName = argv[2];
        searchInFile(regexPattern, fileName);
    } else {
        // Run built-in test cases
        std::cout << "Usage: " << argv[0] << " <regex_pattern> <file_name>" << std::endl;
        std::cout << "Running test cases instead...\n" << std::endl;
        runTestCases();
    }
    return 0;
}