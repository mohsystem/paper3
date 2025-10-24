#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

/**
 * @brief Reads a file and finds lines that start with a number and end with a dot.
 * 
 * @param filePath The path to the file to read.
 * @return std::vector<std::string> A vector containing the matching lines.
 */
std::vector<std::string> findMatchingLines(const std::string& filePath) {
    std::vector<std::string> matchingLines;
    
    // Basic security check to prevent trivial path traversal.
    // For production code, use a library or more robust path validation.
    if (filePath.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid or insecure file path provided." << std::endl;
        return matchingLines;
    }

    // RAII (Resource Acquisition Is Initialization) ensures the file is closed
    // when 'file' goes out of scope, even if exceptions occur.
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'" << std::endl;
        return matchingLines;
    }

    // Regex: ^ (start of line), \d (a digit), .* (any characters), \. (literal dot), $ (end of line)
    // Using a raw string literal R"(...)" to avoid escaping backslashes.
    // This pattern is simple and not vulnerable to ReDoS attacks.
    const std::regex pattern(R"(^\d.*\.$)");
    
    std::string line;
    while (std::getline(file, line)) {
        if (std::regex_match(line, pattern)) {
            matchingLines.push_back(line);
        }
    }

    return matchingLines;
}

// Helper function to create a test file
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
    }
}

int main() {
    const std::string testFileName = "test_cpp.txt";

    // --- Test Cases ---

    // Test Case 1: Standard case with mixed content
    std::cout << "--- Test Case 1: Standard File ---" << std::endl;
    std::string content1 = "1. This is a matching line.\n"
                           "Not a match.\n"
                           "2) Also not a match.\n"
                           "3. Another match here.\n"
                           "4 This line does not end with a dot\n"
                           "This line does not start with a number.\n"
                           "55. This one is also a match.";
    createTestFile(testFileName, content1);
    std::vector<std::string> matches1 = findMatchingLines(testFileName);
    std::cout << "Found " << matches1.size() << " matches:" << std::endl;
    for (const auto& m : matches1) {
        std::cout << m << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Empty file
    std::cout << "--- Test Case 2: Empty File ---" << std::endl;
    createTestFile(testFileName, "");
    std::vector<std::string> matches2 = findMatchingLines(testFileName);
    std::cout << "Found " << matches2.size() << " matches:" << std::endl;
    for (const auto& m : matches2) {
        std::cout << m << std::endl;
    }
    std::cout << std::endl;

    // Test Case 3: File with no matching lines
    std::cout << "--- Test Case 3: No Matches ---" << std::endl;
    std::string content3 = "Hello world\n"
                           "Another line without a dot\n"
                           "Ends with a dot but no number.";
    createTestFile(testFileName, content3);
    std::vector<std::string> matches3 = findMatchingLines(testFileName);
    std::cout << "Found " << matches3.size() << " matches:" << std::endl;
    for (const auto& m : matches3) {
        std::cout << m << std::endl;
    }
    std::cout << std::endl;

    // Test Case 4: File where all lines match
    std::cout << "--- Test Case 4: All Lines Match ---" << std::endl;
    std::string content4 = "1. First line.\n"
                           "2. Second line.\n"
                           "3. Third line.";
    createTestFile(testFileName, content4);
    std::vector<std::string> matches4 = findMatchingLines(testFileName);
    std::cout << "Found " << matches4.size() << " matches:" << std::endl;
    for (const auto& m : matches4) {
        std::cout << m << std::endl;
    }
    std::cout << std::endl;

    // Test Case 5: Non-existent file
    std::cout << "--- Test Case 5: Non-existent File ---" << std::endl;
    std::vector<std::string> matches5 = findMatchingLines("nonexistentfile.txt");
    std::cout << "Found " << matches5.size() << " matches." << std::endl;

    // Cleanup the test file
    std::remove(testFileName.c_str());

    return 0;
}