
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>

std::vector<std::string> matchLines(const std::string& filename) {
    std::vector<std::string> matchedLines;
    std::regex pattern("^\\\\d+.*\\\\.$");
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return matchedLines;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (std::regex_match(line, pattern)) {
            matchedLines.push_back(line);
        }
    }
    
    file.close();
    return matchedLines;
}

int main() {
    // Create test files for demonstration
    std::string testFiles[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    std::vector<std::vector<std::string>> testData = {
        {"123 This is a test.", "456 Another line.", "Not matching", "789 End with dot."},
        {"1 First line.", "Second line", "2 Third line.", "3 Fourth."},
        {"99 Single digit.", "100 Triple digits.", "No number here.", "5."},
        {"0 Zero start.", "abc", "42 The answer.", ""},
        {"7777 Multiple digits here.", "8888.", "Not starting with number.", "9 Final."}
    };
    
    // Create test files
    for (int i = 0; i < 5; i++) {
        std::ofstream outFile(testFiles[i]);
        if (outFile.is_open()) {
            for (const auto& line : testData[i]) {
                outFile << line << std::endl;
            }
            outFile.close();
        }
    }
    
    // Test cases
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << " - File: " << testFiles[i] << std::endl;
        std::vector<std::string> results = matchLines(testFiles[i]);
        std::cout << "Matched lines:" << std::endl;
        for (const auto& line : results) {
            std::cout << "  " << line << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
