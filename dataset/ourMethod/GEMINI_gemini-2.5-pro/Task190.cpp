#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio> // For remove()

std::string transposeFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "Error: File not found.";
    }

    std::vector<std::vector<std::string>> grid;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string word;
        std::vector<std::string> row;
        while (ss >> word) {
            row.push_back(word);
        }
        if (!row.empty()) {
            grid.push_back(row);
        }
    }
    file.close();

    if (grid.empty()) {
        return "";
    }

    size_t rows = grid.size();
    size_t cols = grid[0].size();
    
    std::ostringstream transposedContent;
    for (size_t j = 0; j < cols; ++j) {
        for (size_t i = 0; i < rows; ++i) {
            // Assuming all rows have the same number of columns.
            transposedContent << grid[i][j];
            if (i < rows - 1) {
                transposedContent << " ";
            }
        }
        if (j < cols - 1) {
            transposedContent << "\n";
        }
    }

    return transposedContent.str();
}

void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }
}

void runTestCase(int testNum, const std::string& content, const std::string& expected) {
    const std::string fileName = "file.txt";
    std::cout << "--- Test Case " << testNum << " ---" << std::endl;
    
    createTestFile(fileName, content);
    std::string result = transposeFile(fileName);
    
    std::cout << "Input:" << std::endl;
    std::cout << (content.empty() ? "<empty file>" : content) << std::endl;
    std::cout << "\nOutput:" << std::endl;
    std::cout << result << std::endl;
    std::cout << "\nExpected:" << std::endl;
    std::cout << expected << std::endl;
    std::cout << "Test Passed: " << std::boolalpha << (result == expected) << std::endl;
    
    remove(fileName.c_str());
    
    std::cout << "--------------------" << std::endl;
}

int main() {
    runTestCase(1, "name age\nalice 21\nryan 30", "name alice ryan\nage 21 30");
    runTestCase(2, "a b c\nd e f", "a d\nb e\nc f");
    runTestCase(3, "single_word", "single_word");
    runTestCase(4, "1 2 3 4 5", "1\n2\n3\n4\n5");
    runTestCase(5, "", "");

    std::cout << "--- Test Case 6 (File not found) ---" << std::endl;
    std::string result = transposeFile("non_existent_file.txt");
    std::string expected_error = "Error: File not found.";
    std::cout << "Output:" << std::endl << result << std::endl;
    std::cout << "\nExpected:" << std::endl << expected_error << std::endl;
    std::cout << "Test Passed: " << std::boolalpha << (result == expected_error) << std::endl;
    std::cout << "--------------------" << std::endl;

    return 0;
}