#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

/**
 * @brief Searches a file for lines matching a given regular expression.
 * 
 * @param regexPattern The regular expression pattern as a string.
 * @param fileName The path to the file to be searched.
 */
void searchFileWithRegex(const std::string& regexPattern, const std::string& fileName) {
    // Step 1: Compile the regular expression
    std::regex pattern;
    try {
        pattern = std::regex(regexPattern);
    } catch (const std::regex_error& e) {
        std::cerr << "Error: Invalid regular expression pattern: " << e.what() << std::endl;
        return;
    }

    // Step 2: Open and read the file
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << fileName << std::endl;
        return;
    }

    // Step 3: Search line by line
    std::string line;
    int lineNumber = 1;
    bool found = false;
    while (std::getline(file, line)) {
        if (std::regex_search(line, pattern)) {
            std::cout << "Match found on line " << lineNumber << ": " << line << std::endl;
            found = true;
        }
        lineNumber++;
    }

    if (!found) {
        std::cout << "No matches found for pattern '" << regexPattern << "' in file '" << fileName << "'." << std::endl;
    }

    // File is closed automatically when 'file' goes out of scope (RAII)
}

void runTests() {
    const std::string testFileName = "test_file.txt";
    // Create a temporary test file
    std::ofstream outFile(testFileName);
    if (!outFile) {
        std::cerr << "Failed to create test file." << std::endl;
        return;
    }
    outFile << "Hello world, this is a test file.\n";
    outFile << "The quick brown fox jumps over the lazy dog.\n";
    outFile << "123-456-7890 is a phone number.\n";
    outFile << "Another line with the word world.\n";
    outFile << "Email: test@example.com\n";
    outFile << "invalid-email@.com\n";
    outFile.close();

    struct TestCase {
        std::string pattern;
        std::string description;
    };

    std::vector<TestCase> testCases = {
        {"world", "Test Case 1: Simple word match"},
        {"^[A-Z]", "Test Case 2: Match lines starting with a capital letter"},
        {"\\d{3}-\\d{3}-\\d{4}", "Test Case 3: Match a phone number format"},
        {"fox|dog", "Test Case 4: Match using alternation"},
        {"[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", "Test Case 5: Match a valid email address"}
    };

    for (const auto& testCase : testCases) {
        std::cout << "\n--- " << testCase.description << " ---" << std::endl;
        std::cout << "Pattern: " << testCase.pattern << std::endl;
        std::cout << "------------------------------------" << std::endl;
        searchFileWithRegex(testCase.pattern, testFileName);
    }

    // Clean up the test file
    std::remove(testFileName.c_str());
}

int main(int argc, char* argv[]) {
    // Check if command line arguments are provided
    if (argc == 3) {
        std::cout << "--- Running with Command Line Arguments ---" << std::endl;
        std::string regex = argv[1];
        std::string file = argv[2];
        std::cout << "Pattern: " << regex << std::endl;
        std::cout << "File: " << file << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        searchFileWithRegex(regex, file);
    } else {
        std::cout << "Usage: " << argv[0] << " <regex_pattern> <file_name>" << std::endl;
        std::cout << "Running built-in test cases as no arguments were provided." << std::endl;
        runTests();
    }
    return 0;
}