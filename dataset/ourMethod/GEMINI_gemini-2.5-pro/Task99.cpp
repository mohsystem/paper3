#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cstdio> // For std::remove

/**
 * Reads a file and returns a vector of lines that start with numbers and end with a dot.
 *
 * @param filePath The path to the file to read.
 * @return A vector of strings containing the matching lines.
 */
std::vector<std::string> findMatchingLines(const std::string& filePath) {
    std::vector<std::string> matchingLines;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        // Return empty vector if file cannot be opened
        return matchingLines;
    }

    // Regex: ^\d+.*\.$.
    // ^     - Start of the line
    // \d+   - One or more digits (in ECMAScript grammar, which is default in C++)
    // .*    - Any character, zero or more times
    // \.    - A literal dot
    // $     - End of the line
    try {
        std::regex pattern("^\\d+.*\\.$");
        std::string line;
        while (std::getline(file, line)) {
            // On some systems, getline may include carriage returns.
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (std::regex_match(line, pattern)) {
                matchingLines.push_back(line);
            }
        }
    } catch (const std::regex_error& e) {
        std::cerr << "Regex error: " << e.what() << std::endl;
    }
    
    file.close();
    return matchingLines;
}

void runTestCase(int testNum, const std::string& fileName, const std::string& fileContent) {
    std::cout << "--- Test Case " << testNum << " ---" << std::endl;
    
    // Create and write to the test file
    std::ofstream outFile(fileName);
    if (!outFile) {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
        return;
    }
    outFile << fileContent;
    outFile.close();

    // Run the function and print results
    std::cout << "File content:\n\"\"\"\n" << fileContent << "\"\"\"" << std::endl;
    std::vector<std::string> results = findMatchingLines(fileName);
    std::cout << "Matching lines found: " << results.size() << std::endl;
    for (const auto& line : results) {
        std::cout << " > " << line << std::endl;
    }

    // Clean up the test file
    std::remove(fileName.c_str());
    std::cout << std::endl;
}

int main() {
    const std::string fileName = "test_file_cpp.txt";

    // Test Case 1: Mixed content
    const std::string content1 = "123 this is a test.\n"
                                 "456 another line that matches.\n"
                                 "not a match\n"
                                 "789 this line does not end with a dot\n"
                                 "and another line that does not start with a number.\n"
                                 "999.\n"
                                 "1.";
    runTestCase(1, fileName, content1);

    // Test Case 2: All lines match
    const std::string content2 = "1. First line.\n"
                                 "20. Second line.\n"
                                 "300. Third line.";
    runTestCase(2, fileName, content2);
    
    // Test Case 3: No lines match
    const std::string content3 = "This starts with text.\n"
                                 "123 this ends with no dot\n"
                                 "Ends with a dot but no number.";
    runTestCase(3, fileName, content3);
    
    // Test Case 4: Empty file
    const std::string content4 = "";
    runTestCase(4, fileName, content4);
    
    // Test Case 5: Lines with special regex characters
    const std::string content5 = "123.*.\n"
                                 "456?[]\\d+.\n"
                                 "abc.";
    runTestCase(5, fileName, content5);

    return 0;
}