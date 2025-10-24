
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

std::vector<std::string> matchLines(const std::string& filename) {
    std::vector<std::string> matchedLines;
    std::regex pattern("^\\\\d+.*\\\\.$");
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading file: " << filename << std::endl;
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
    // Test cases - creating test files
    std::vector<std::string> testFiles = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    
    // Test case 1: Simple numbered list
    std::ofstream f1(testFiles[0]);
    f1 << "1 This is a sentence.\\n";
    f1 << "2 Another line.\\n";
    f1 << "Not matching\\n";
    f1 << "3 Third line.\\n";
    f1.close();
    
    // Test case 2: Multi-digit numbers
    std::ofstream f2(testFiles[1]);
    f2 << "123 Large number line.\\n";
    f2 << "456 Another large number.\\n";
    f2 << "No number here.\\n";
    f2.close();
    
    // Test case 3: Lines without dots
    std::ofstream f3(testFiles[2]);
    f3 << "1 This has no dot\\n";
    f3 << "2 This has a dot.\\n";
    f3 << "3 No dot here\\n";
    f3.close();
    
    // Test case 4: Mixed content
    std::ofstream f4(testFiles[3]);
    f4 << "99 Special characters !@#$.\\n";
    f4 << "Start with text 100.\\n";
    f4 << "0 Zero starts this.\\n";
    f4.close();
    
    // Test case 5: Empty and edge cases
    std::ofstream f5(testFiles[4]);
    f5 << "1.\\n";
    f5 << "2\\n";
    f5 << ".\\n";
    f5 << "12345 Multiple words here.\\n";
    f5.close();
    
    // Execute test cases
    for (size_t i = 0; i < testFiles.size(); i++) {
        std::cout << "Test case " << (i + 1) << " - File: " << testFiles[i] << std::endl;
        std::vector<std::string> results = matchLines(testFiles[i]);
        std::cout << "Matched lines: ";
        for (const auto& line : results) {
            std::cout << "\\"" << line << "\\" ";
        }
        std::cout << std::endl << std::endl;
    }
    
    return 0;
}
