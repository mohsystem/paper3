#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

/**
 * @brief Reads a file containing key-value pairs separated by a colon,
 * sorts them by key, and prints the result.
 *
 * @param filePath The path to the input file.
 */
void sortKeyValueFile(const std::string& filePath) {
    // std::ifstream's destructor will automatically close the file (RAII).
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'" << std::endl;
        return;
    }

    // std::map automatically keeps its elements sorted by key.
    std::map<std::string, std::string> records;
    std::string line;

    while (std::getline(file, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // Basic trim for key and value
            key.erase(0, key.find_first_not_of(" \t\n\r"));
            key.erase(key.find_last_not_of(" \t\n\r") + 1);
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            value.erase(value.find_last_not_of(" \t\n\r") + 1);
            
            if (!key.empty()) { // Ensure key is not empty
                records[key] = value;
            }
        }
    }

    // Print the sorted records
    for (const auto& pair : records) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

/**
 * @brief Helper function to create a test file.
 */
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
    }
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Normal file with unsorted data
    std::cout << "--- Test Case 1: Normal File ---" << std::endl;
    std::string testFile1 = "test1.txt";
    createTestFile(testFile1, "banana:fruit\napple:fruit\ncarrot:vegetable\nzucchini:vegetable\n");
    sortKeyValueFile(testFile1);
    std::cout << std::endl;

    // Test Case 2: Empty file
    std::cout << "--- Test Case 2: Empty File ---" << std::endl;
    std::string testFile2 = "test2.txt";
    createTestFile(testFile2, "");
    sortKeyValueFile(testFile2);
    std::cout << std::endl;

    // Test Case 3: File with malformed lines
    std::cout << "--- Test Case 3: Malformed Lines ---" << std::endl;
    std::string testFile3 = "test3.txt";
    createTestFile(testFile3, "one:1\ntwo_malformed\nthree:3\n:missing_key\nfour::4\n");
    sortKeyValueFile(testFile3);
    std::cout << std::endl;

    // Test Case 4: Non-existent file
    std::cout << "--- Test Case 4: Non-existent File ---" << std::endl;
    std::string testFile4 = "non_existent_file.txt";
    sortKeyValueFile(testFile4);
    std::cout << std::endl;

    // Test Case 5: File with duplicate keys (last one should win)
    std::cout << "--- Test Case 5: Duplicate Keys ---" << std::endl;
    std::string testFile5 = "test5.txt";
    createTestFile(testFile5, "key1:valueA\nkey2:valueB\nkey1:valueC\n");
    sortKeyValueFile(testFile5);
    std::cout << std::endl;

    // Cleanup test files
    remove(testFile1.c_str());
    remove(testFile2.c_str());
    remove(testFile3.c_str());
    remove(testFile5.c_str());
    
    return 0;
}