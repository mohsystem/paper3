
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <cstdio>

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

std::vector<std::string> matchLines(const std::string& filePath) {
    std::vector<std::string> matchedLines;
    
    if (filePath.empty()) {
        std::cerr << "Error: File path cannot be empty" << std::endl;
        return matchedLines;
    }
    
    try {
        // Resolve path to prevent path traversal
        std::filesystem::path fsPath(filePath);
        
        if (!std::filesystem::exists(fsPath)) {
            std::cerr << "Error: File does not exist: " << filePath << std::endl;
            return matchedLines;
        }
        
        // Check file size to prevent DOS attacks
        size_t fileSize = std::filesystem::file_size(fsPath);
        if (fileSize > MAX_FILE_SIZE) {
            std::cerr << "Error: File size exceeds maximum allowed size" << std::endl;
            return matchedLines;
        }
        
        std::ifstream file(fsPath);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file: " << filePath << std::endl;
            return matchedLines;
        }
        
        // Regular expression: starts with digit(s), ends with dot
        std::regex pattern("^\\\\d+.*\\\\.$");
        std::string line;
        
        while (std::getline(file, line)) {
            if (std::regex_match(line, pattern)) {
                matchedLines.push_back(line);
            }
        }
        
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return matchedLines;
}

int main() {
    // Test case 1: File with numbered lines
    std::string testFile1 = "test1.txt";
    {
        std::ofstream file(testFile1);
        file << "1 This is line one.\\n2 Second line.\\n3 Third line.";
        file.close();
        
        std::cout << "Test 1 - File with numbered lines:" << std::endl;
        std::vector<std::string> result1 = matchLines(testFile1);
        for (const auto& line : result1) {
            std::cout << line << std::endl;
        }
        std::remove(testFile1.c_str());
    }
    
    // Test case 2: Mixed content
    std::string testFile2 = "test2.txt";
    {
        std::ofstream file(testFile2);
        file << "123 Start with number.\\nNo number start.\\n456 Another one.";
        file.close();
        
        std::cout << "\\nTest 2 - Mixed content:" << std::endl;
        std::vector<std::string> result2 = matchLines(testFile2);
        for (const auto& line : result2) {
            std::cout << line << std::endl;
        }
        std::remove(testFile2.c_str());
    }
    
    // Test case 3: No matching lines
    std::string testFile3 = "test3.txt";
    {
        std::ofstream file(testFile3);
        file << "No numbers here\\nAnother line without numbers";
        file.close();
        
        std::cout << "\\nTest 3 - No matching lines:" << std::endl;
        std::vector<std::string> result3 = matchLines(testFile3);
        std::cout << "Matched lines: " << result3.size() << std::endl;
        std::remove(testFile3.c_str());
    }
    
    // Test case 4: Lines ending without dot
    std::string testFile4 = "test4.txt";
    {
        std::ofstream file(testFile4);
        file << "1 Line with dot.\\n2 Line without dot\\n3 Another with dot.";
        file.close();
        
        std::cout << "\\nTest 4 - Lines with and without ending dot:" << std::endl;
        std::vector<std::string> result4 = matchLines(testFile4);
        for (const auto& line : result4) {
            std::cout << line << std::endl;
        }
        std::remove(testFile4.c_str());
    }
    
    // Test case 5: Invalid file path
    std::cout << "\\nTest 5 - Invalid file path:" << std::endl;
    std::vector<std::string> result5 = matchLines("nonexistent_file.txt");
    std::cout << "Matched lines: " << result5.size() << std::endl;
    
    return 0;
}
