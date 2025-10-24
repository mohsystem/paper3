#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

/**
 * Transposes the content of a text file.
 *
 * @param filePath The path to the input text file.
 */
void transposeFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::vector<std::vector<std::string>> matrix;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string word;
        while (ss >> word) {
            row.push_back(word);
        }
        if (!row.empty()) {
            matrix.push_back(row);
        }
    }
    file.close();

    if (matrix.empty() || matrix[0].empty()) {
        return;
    }

    size_t numRows = matrix.size();
    size_t numCols = matrix[0].size();

    for (size_t j = 0; j < numCols; ++j) {
        for (size_t i = 0; i < numRows; ++i) {
            std::cout << matrix[i][j] << (i == numRows - 1 ? "" : " ");
        }
        std::cout << std::endl;
    }
}

/**
 * Helper function to create a test file with given content.
 */
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }
}

int main() {
    std::vector<std::string> testContents = {
        // Test Case 1: Example from prompt
        "name age\nalice 21\nryan 30",
        // Test Case 2: Single row
        "a b c d",
        // Test Case 3: Single column
        "a\nb\nc",
        // Test Case 4: 4x4 matrix
        "1 2 3 4\n5 6 7 8\n9 10 11 12\n13 14 15 16",
        // Test Case 5: 3x3 with strings
        "first middle last\njohn f kennedy\nmartin luther king"
    };

    std::string fileName = "file.txt";
    for (size_t i = 0; i < testContents.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input:" << std::endl;
        std::cout << testContents[i] << std::endl;
        std::cout << "\nOutput:" << std::endl;
        
        createTestFile(fileName, testContents[i]);
        transposeFile(fileName);
        std::cout << std::endl;
    }

    // Clean up the created file
    remove(fileName.c_str());

    return 0;
}