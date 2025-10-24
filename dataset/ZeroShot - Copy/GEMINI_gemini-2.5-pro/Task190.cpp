#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

/**
 * Reads a space-delimited text file, transposes its content, and returns the result.
 * Assumes all rows have the same number of columns.
 *
 * @param filename The path to the input file.
 * @return A string containing the transposed content.
 */
std::string transposeFileContent(const std::string& filename) {
    // Secure: std::ifstream's destructor will automatically close the file (RAII).
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Secure: Don't expose full path in error messages in production.
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
    }

    std::vector<std::vector<std::string>> grid;
    std::string line;
    
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string token;
        // Using getline with a delimiter is a robust way to split strings,
        // handling various spacing scenarios.
        while(std::getline(ss, token, ' ')) {
            row.push_back(token);
        }
        grid.push_back(row);
    }
    
    if (grid.empty() || grid[0].empty()) {
        return "";
    }

    size_t numRows = grid.size();
    size_t numCols = grid[0].size();
    
    // Secure: For robustness, validate the assumption that all rows have the same number of columns.
    for(size_t i = 1; i < numRows; ++i) {
        if (grid[i].size() != numCols) {
            std::cerr << "Error: Inconsistent number of columns in file. Aborting." << std::endl;
            return "";
        }
    }

    std::stringstream result;
    for (size_t j = 0; j < numCols; ++j) {
        for (size_t i = 0; i < numRows; ++i) {
            result << grid[i][j];
            if (i < numRows - 1) {
                result << " ";
            }
        }
        if (j < numCols - 1) {
            result << "\n";
        }
    }

    return result.str();
}

void runTestCase(int testNum, const std::string& content, const std::string& expected) {
    const std::string filename = "file.txt";
    std::cout << "--- Test Case " << testNum << " ---" << std::endl;

    // Create test file
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }

    std::string result = transposeFileContent(filename);
    
    std::cout << "Input:\n" << (content.empty() ? "(empty)" : content) << std::endl;
    std::cout << "\nOutput:\n" << (result.empty() ? "(empty)" : result) << std::endl;
    std::cout << "\nExpected:\n" << (expected.empty() ? "(empty)" : expected) << std::endl;
    std::cout << "\nResult matches expected: " << (result == expected ? "true" : "false") << std::endl;

    // Clean up
    remove(filename.c_str());
    std::cout << "---------------------\n" << std::endl;
}

int main() {
    runTestCase(1, "name age\nalice 21\nryan 30", "name alice ryan\nage 21 30");
    runTestCase(2, "a b c d", "a\nb\nc\nd");
    runTestCase(3, "x\ny\nz", "x y z");
    runTestCase(4, "1 2 3\n4 5 6\n7 8 9", "1 4 7\n2 5 8\n3 6 9");
    runTestCase(5, "", "");

    return 0;
}