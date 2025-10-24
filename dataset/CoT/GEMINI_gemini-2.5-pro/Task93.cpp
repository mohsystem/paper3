#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility> // For std::pair
#include <algorithm> // For std::sort
#include <cctype> // for isspace

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

/**
 * Reads a file containing key-value pairs, sorts them by key, and returns them.
 *
 * @param filePath The path to the file.
 * @return A vector of pairs representing the sorted key-value records.
 *         Returns an empty vector if the file cannot be opened.
 */
std::vector<std::pair<std::string, std::string>> sortFileRecords(const std::string& filePath) {
    std::vector<std::pair<std::string, std::string>> records;
    // ifstream's destructor will automatically close the file (RAII).
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'" << std::endl;
        return records; // Return empty vector on error
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = trim(line.substr(0, delimiterPos));
            std::string value = trim(line.substr(delimiterPos + 1));
            
            if (!key.empty()) { // Ensure key is not empty
                records.emplace_back(key, value);
            }
        }
    }

    // Sort the vector of pairs based on the key (the first element of the pair).
    std::sort(records.begin(), records.end(), 
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    return records;
}

// Main function with test cases
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
    }
}

void printRecords(const std::vector<std::pair<std::string, std::string>>& records) {
    if (records.empty()) {
        std::cout << "No valid records found or processed." << std::endl;
    } else {
        for (const auto& record : records) {
            std::cout << record.first << ": " << record.second << std::endl;
        }
    }
}

int main() {
    std::string testFileName = "test_records.txt";

    // Test Case 1: Standard case with unsorted records
    std::cout << "--- Test Case 1: Standard unsorted file ---" << std::endl;
    createTestFile(testFileName, "banana: 10\napple: 5\ncherry: 20\ndate: 15");
    auto sortedRecords1 = sortFileRecords(testFileName);
    printRecords(sortedRecords1);
    std::cout << std::endl;

    // Test Case 2: File with malformed lines, empty lines, and whitespace
    std::cout << "--- Test Case 2: File with malformed lines and whitespace ---" << std::endl;
    createTestFile(testFileName, "  fig: 30  \n\ngrape: 25\njust_a_key\n:empty_key\nelderberry:");
    auto sortedRecords2 = sortFileRecords(testFileName);
    printRecords(sortedRecords2);
    std::cout << std::endl;

    // Test Case 3: Empty file
    std::cout << "--- Test Case 3: Empty file ---" << std::endl;
    createTestFile(testFileName, "");
    auto sortedRecords3 = sortFileRecords(testFileName);
    printRecords(sortedRecords3);
    std::cout << std::endl;

    // Test Case 4: Non-existent file
    std::cout << "--- Test Case 4: Non-existent file ---" << std::endl;
    remove(testFileName.c_str()); // Ensure file does not exist
    auto sortedRecords4 = sortFileRecords(testFileName);
    printRecords(sortedRecords4);
    std::cout << std::endl;

    // Test Case 5: Already sorted file
    std::cout << "--- Test Case 5: Already sorted file ---" << std::endl;
    createTestFile(testFileName, "alpha: 1\nbeta: 2\ngamma: 3");
    auto sortedRecords5 = sortFileRecords(testFileName);
    printRecords(sortedRecords5);
    std::cout << std::endl;

    // Cleanup the test file
    remove(testFileName.c_str());

    return 0;
}