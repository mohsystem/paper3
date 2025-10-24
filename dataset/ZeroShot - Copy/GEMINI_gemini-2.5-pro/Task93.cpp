#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

/**
 * Reads a file containing key-value pairs separated by a colon, sorts them by key.
 * This implementation uses std::map to automatically keep the keys sorted.
 *
 * @param filePath The path to the input file.
 * @return A std::map containing the sorted key-value pairs.
 */
std::map<std::string, std::string> readAndSortFile(const std::string& filePath) {
    std::map<std::string, std::string> sortedRecords;
    
    // ifstream's destructor will automatically close the file (RAII).
    std::ifstream inFile(filePath);

    if (!inFile.is_open()) {
        throw std::runtime_error("Error: Could not open file " + filePath);
    }

    std::string line;
    while (std::getline(inFile, line)) {
        // Find the position of the first colon
        size_t delimiterPos = line.find(':');
        
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            if (!key.empty()) {
                sortedRecords[key] = value;
            }
        }
        // Malformed lines (without a colon) or empty keys are ignored.
    }
    
    return sortedRecords;
}

// Helper function to run a single test case
void runTestCase(int testNum, const std::string& content) {
    std::string testFileName = "test_case_" + std::to_string(testNum) + ".txt";
    std::cout << "--- Running Test Case " << testNum << " ---" << std::endl;

    try {
        // 1. Create and write to the test file
        {
            std::ofstream outFile(testFileName);
            if (!outFile) {
                throw std::runtime_error("Failed to create test file.");
            }
            outFile << content;
        } // outFile is closed here by RAII

        // 2. Process the file
        std::map<std::string, std::string> sortedRecords = readAndSortFile(testFileName);

        // 3. Print the results
        if (sortedRecords.empty()) {
            std::cout << "No valid records found or file was empty." << std::endl;
        } else {
            for (const auto& pair : sortedRecords) {
                std::cout << pair.first << ":" << pair.second << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // 4. Clean up the test file
    remove(testFileName.c_str());
    std::cout << std::endl;
}

int main() {
    std::vector<std::string> testContents = {
        // Test Case 1: Standard case with mixed order
        "banana:yellow\napple:red\norange:orange\ngrape:purple",
        // Test Case 2: Empty file
        "",
        // Test Case 3: File with malformed lines, empty lines, and extra whitespace
        "name: Alice\n\nage: 30\noccupation :Software Engineer\nlocation\n\ncity:New York",
        // Test Case 4: Case sensitivity test
        "Apple:fruit\napple:fruit\nZebra:animal\nzoo:place",
        // Test Case 5: Special characters in values and keys
        "url:http://example.com?a=1&b=2\n$pecial-key:value with spaces\nemail:test@example.com"
    };
    
    for (int i = 0; i < testContents.size(); ++i) {
        runTestCase(i + 1, testContents[i]);
    }

    return 0;
}