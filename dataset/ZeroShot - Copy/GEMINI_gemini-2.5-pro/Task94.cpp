#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm> // For std::find_if
#include <cctype>    // For std::isspace

// Helper function to trim whitespace from the start of a string
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Helper function to trim whitespace from the end of a string
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Helper function to trim whitespace from both ends of a string
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

/**
 * Reads a file containing key-value pairs separated by a colon, sorts them by key.
 *
 * @param filePath The path to the file to be read.
 * @return A std::map containing the key-value pairs sorted by key.
 */
std::map<std::string, std::string> sortFileRecords(const std::string& filePath) {
    // std::map automatically keeps its elements sorted by key.
    std::map<std::string, std::string> sortedRecords;
    
    // std::ifstream's destructor will close the file automatically (RAII).
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'" << std::endl;
        return sortedRecords; // Return empty map
    }
    
    std::string line;
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) {
            continue; // Skip empty lines
        }
        
        // Find the first occurrence of the delimiter ':'
        size_t delimiterPos = line.find(':');
        
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            
            trim(key);
            trim(value);
            
            if (!key.empty()) { // Ensure key is not empty
                sortedRecords[key] = value;
            }
        } else {
            std::cerr << "Warning: Skipping malformed line: " << line << std::endl;
        }
    }
    
    return sortedRecords;
}

// --- Test Cases ---

void setupTestFiles() {
    // Test Case 1
    std::ofstream("test_ok.txt") << "zulu:last\nalpha:first\ncharlie:third\nbravo:second\n";
    // Test Case 2
    std::ofstream("test_malformed.txt") << "key1:value1\nmalformedline\nkey2:value2\nanother:malformed:line\n";
    // Test Case 3
    std::ofstream("test_empty.txt").close();
    // Test Case 4
    std::ofstream("test_blank_lines.txt") << "keyA:valueA\n\n  \nkeyC:valueC\nkeyB:valueB\n";
}

void cleanupTestFiles() {
    remove("test_ok.txt");
    remove("test_malformed.txt");
    remove("test_empty.txt");
    remove("test_blank_lines.txt");
}

int main() {
    setupTestFiles();
    
    std::vector<std::string> testFiles = {
        "test_ok.txt",
        "test_malformed.txt",
        "test_empty.txt",
        "test_blank_lines.txt",
        "non_existent_file.txt"
    };
    
    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::cout << "--- Test Case " << i + 1 << ": Processing " << testFiles[i] << " ---" << std::endl;
        std::map<std::string, std::string> result = sortFileRecords(testFiles[i]);
        if (result.empty()) {
            std::cout << "Result is empty (as expected for empty, malformed, or non-existent files)." << std::endl;
        } else {
            for (const auto& pair : result) {
                std::cout << pair.first << ":" << pair.second << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    cleanupTestFiles();
    
    return 0;
}