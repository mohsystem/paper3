
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>

// Transpose the content of a file
// Security: Validates file access, bounds checks all operations, no dynamic code execution
std::string transposeFile(const std::string& filename) {
    // Security: Validate filename is not empty and doesn't contain path traversal
    if (filename.empty() || filename.find("..") != std::string::npos) {
        return ""; // Fail closed on invalid input
    }
    
    std::ifstream file(filename);
    // Security: Check file open success before proceeding
    if (!file.is_open()) {
        return ""; // Fail closed if file cannot be opened
    }
    
    std::vector<std::vector<std::string>> matrix;
    std::string line;
    
    // Security: Limit maximum file size to prevent memory exhaustion (10MB)
    const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
    size_t totalSize = 0;
    
    // Read all lines and split into columns
    while (std::getline(file, line)) {
        totalSize += line.size();
        // Security: Check for excessive file size
        if (totalSize > MAX_FILE_SIZE) {
            file.close();
            return ""; // Fail closed on oversized input
        }
        
        std::vector<std::string> row;
        std::istringstream iss(line);
        std::string token;
        
        // Security: Limit maximum columns to prevent resource exhaustion
        const size_t MAX_COLUMNS = 10000;
        size_t colCount = 0;
        
        while (iss >> token) {
            colCount++;
            // Security: Validate column count
            if (colCount > MAX_COLUMNS) {
                file.close();
                return ""; // Fail closed on excessive columns
            }
            row.push_back(token);
        }
        
        // Security: Only add non-empty rows
        if (!row.empty()) {
            matrix.push_back(row);
        }
    }
    
    file.close();
    
    // Security: Validate matrix is not empty
    if (matrix.empty()) {
        return "";
    }
    
    // Get the number of columns from the first row
    size_t numCols = matrix[0].size();
    
    // Transpose the matrix
    std::ostringstream result;
    for (size_t col = 0; col < numCols; ++col) {
        for (size_t row = 0; row < matrix.size(); ++row) {
            // Security: Bounds check before accessing matrix element
            if (col < matrix[row].size()) {
                result << matrix[row][col];
                if (row < matrix.size() - 1) {
                    result << " ";
                }
            }
        }
        if (col < numCols - 1) {
            result << "\\n";
        }
    }
    
    return result.str();
}

int main() {
    // Test case 1: Standard input with multiple rows and columns
    {
        std::ofstream out("file.txt");
        out << "name age\\n";
        out << "alice 21\\n";
        out << "ryan 30\\n";
        out.close();
        
        std::string result = transposeFile("file.txt");
        std::cout << "Test 1:\\n" << result << "\\n\\n";
    }
    
    // Test case 2: Single row
    {
        std::ofstream out("file.txt");
        out << "a b c d\\n";
        out.close();
        
        std::string result = transposeFile("file.txt");
        std::cout << "Test 2:\\n" << result << "\\n\\n";
    }
    
    // Test case 3: Single column
    {
        std::ofstream out("file.txt");
        out << "a\\n";
        out << "b\\n";
        out << "c\\n";
        out.close();
        
        std::string result = transposeFile("file.txt");
        std::cout << "Test 3:\\n" << result << "\\n\\n";
    }
    
    // Test case 4: 3x3 matrix
    {
        std::ofstream out("file.txt");
        out << "1 2 3\\n";
        out << "4 5 6\\n";
        out << "7 8 9\\n";
        out.close();
        
        std::string result = transposeFile("file.txt");
        std::cout << "Test 4:\\n" << result << "\\n\\n";
    }
    
    // Test case 5: Non-existent file (security test)
    {
        std::string result = transposeFile("nonexistent.txt");
        std::cout << "Test 5 (non-existent file): " 
                  << (result.empty() ? "Handled correctly" : "Error") << "\\n\\n";
    }
    
    return 0;
}
