#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cstdio> // For std::remove

/**
 * @brief Reads a file and finds lines that start with a number and end with a dot.
 * 
 * @param filePath The path to the input file.
 * @return A vector of strings, where each string is a matching line.
 */
std::vector<std::string> findMatchingLines(const std::string& filePath) {
    std::vector<std::string> matchingLines;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return matchingLines;
    }

    // Regex: starts with a digit (^\\d), followed by anything (.*), ends with a dot (\\.$).
    // The backslashes are escaped for the C++ string literal.
    std::regex pattern("^\\d.*\\.$");
    std::string line;

    while (std::getline(file, line)) {
        if (std::regex_match(line, pattern)) {
            matchingLines.push_back(line);
        }
    }

    file.close();
    return matchingLines;
}

/**
 * @brief Helper function to run a single test case.
 * 
 * @param fileName The name of the temporary file to create.
 * @param content The content to write to the file.
 */
void runTestCase(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (!outFile) {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
        return;
    }
    outFile << content;
    outFile.close();

    std::cout << "\n--- Testing with file: " << fileName << " ---" << std::endl;
    std::cout << "Content:\n\"";
    for (char c : content) {
        if (c == '\n') {
            std::cout << "\\n";
        } else {
            std::cout << c;
        }
    }
    std::cout << "\"" << std::endl;

    std::vector<std::string> result = findMatchingLines(fileName);
    
    std::cout << "Result:" << std::endl;
    if (result.empty()) {
        std::cout << "(No matching lines found)" << std::endl;
    } else {
        for (const auto& line : result) {
            std::cout << line << std::endl;
        }
    }

    std::remove(fileName.c_str());
}

int main() {
    // Test Case 1: A file with a mix of valid and invalid lines.
    runTestCase("test_cpp_1.txt",
                "1. This is a valid line.\n"
                "This line is not valid.\n"
                "2. This is also a valid line.\n"
                "3 This line is not valid, no dot at the end\n"
                "4.Valid line.\n"
                "5.\n"
                "Invalid line.\n"
                "6. Another. valid. line.");

    // Test Case 2: An empty file.
    runTestCase("test_cpp_2.txt", "");

    // Test Case 3: A file with no matching lines.
    runTestCase("test_cpp_3.txt", "Hello world\nThis is a test\nNo lines match here");

    // Test Case 4: A file where all lines match.
    runTestCase("test_cpp_4.txt", "1. First.\n2. Second.\n3. Third.");

    // Test Case 5: A file with special characters and multiple digits.
    runTestCase("test_cpp_5.txt", "123. Special chars !@#$%^&*().\nAnother line\n45.Ends with a dot.");
    
    return 0;
}