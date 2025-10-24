#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <cstdio> // For std::remove

// Represents a single key-value record
using Record = std::pair<std::string, std::string>;

// Trims whitespace from the start of a string (in-place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trims whitespace from the end of a string (in-place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Trims whitespace from both ends of a string (in-place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

/**
 * Reads key-value pairs from a file, sorts them by key, and returns them.
 *
 * @param filePath The path to the file. Must be a relative path.
 * @return A vector of sorted Record pairs.
 * @throws std::invalid_argument for invalid file paths.
 * @throws std::runtime_error for file I/O errors.
 */
std::vector<Record> sortRecordsFromFile(const std::string& filePath) {
    // Rule #7: Validate and sanitize all external input used in path construction.
    if (filePath.empty() || filePath.find("..") != std::string::npos || 
        filePath.front() == '/' || filePath.front() == '\\') {
        throw std::invalid_argument("Invalid file path: Only relative paths within the current directory are allowed.");
    }
    
    // Rule #9: Use RAII for resource management (ifstream handles file closing).
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::vector<Record> records;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        trim(line);

        if (line.empty() || line.front() == '#') {
            continue;
        }

        size_t separator_pos = line.find(':');
        // Rule #4: Validate that input conforms to the expected format.
        if (separator_pos == std::string::npos || separator_pos == 0 || separator_pos == line.length() - 1) {
            std::cerr << "Warning: Malformed line " << lineNumber << ": " << line << std::endl;
            continue;
        }

        std::string key = line.substr(0, separator_pos);
        std::string value = line.substr(separator_pos + 1);

        trim(key);
        trim(value);

        if (key.empty()) {
            std::cerr << "Warning: Malformed line (empty key) " << lineNumber << ": " << line << std::endl;
            continue;
        }

        records.emplace_back(key, value);
    }

    // Sort records based on the key
    std::sort(records.begin(), records.end(), 
              [](const Record& a, const Record& b) {
                  return a.first < b.first;
              });

    return records;
}

void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream file(fileName);
    if (file.is_open()) {
        file << content;
    } else {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
    }
}

int main() {
    const char* testFiles[] = {"test1.txt", "test2.txt", "test3.txt"};

    // Test Case 1: Normal file
    createTestFile("test1.txt", "c:3\n a: 1 \n b:2\n");
    // Test Case 2: Empty file
    createTestFile("test2.txt", "");
    // Test Case 3: File with malformed lines
    createTestFile("test3.txt", "apple:red\n:empty_key\nbanana:\n# a comment\nmalformed line\norange: fruit\n");

    const char* testCases[] = {
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "non_existent_file.txt",
        "../forbidden.txt"
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": " << testCases[i] << " ---" << std::endl;
        try {
            std::vector<Record> sortedRecords = sortRecordsFromFile(testCases[i]);
            if (sortedRecords.empty()) {
                std::cout << "No records found or file was empty/invalid." << std::endl;
            } else {
                for (const auto& record : sortedRecords) {
                    std::cout << record.first << ":" << record.second << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }

    // Cleanup
    for (const auto& fileName : testFiles) {
        std::remove(fileName);
    }

    return 0;
}