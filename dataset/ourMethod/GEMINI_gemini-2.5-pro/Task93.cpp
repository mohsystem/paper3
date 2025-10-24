#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <stdexcept>

// Helper function to trim whitespace from a string
std::string trim(const std::string& str) {
    const std::string WHITESPACE = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}

/**
 * Reads key-value pairs from a file, with each line formatted as "key:value".
 * Malformed lines are skipped.
 *
 * @param filename The path to the file to read.
 * @return A vector of key-value pairs.
 */
std::vector<std::pair<std::string, std::string>> readFileRecords(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> records;
    // Use RAII for automatic file resource management.
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return records;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Validate input format.
        size_t separator_pos = line.find(':');
        if (separator_pos != std::string::npos) {
            std::string key = trim(line.substr(0, separator_pos));
            std::string value = trim(line.substr(separator_pos + 1));

            if (!key.empty() && !value.empty()) {
                records.push_back({key, value});
            }
        }
        // Silently ignore malformed lines.
    }

    return records;
}

/**
 * Sorts a vector of key-value pairs by key and prints them.
 *
 * @param records The vector of records to sort and print.
 */
void sortAndPrintRecords(std::vector<std::pair<std::string, std::string>>& records) {
    if (records.empty()) {
        std::cout << "No records to display." << std::endl;
        return;
    }

    // Sort records by key (default pair sorting behavior).
    std::sort(records.begin(), records.end());

    // Print sorted records.
    for (const auto& record : records) {
        std::cout << record.first << ": " << record.second << std::endl;
    }
}

// Helper function to create a test file
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create test file: " + filename);
    }
    file << content;
}

void runTestCase(int testNum, const std::string& filename, const std::string& content) {
    std::cout << "----- Test Case " << testNum << ": " << filename << " -----" << std::endl;
    try {
        // Setup
        createTestFile(filename, content);

        // Execute
        auto records = readFileRecords(filename);

        // Process and Print
        sortAndPrintRecords(records);

    } catch (const std::exception& e) {
        std::cerr << "Test case failed: " << e.what() << std::endl;
    }
    // Teardown
    remove(filename.c_str());
    std::cout << std::endl;
}

int main() {
    runTestCase(1, "test1.txt", "c:3\na:1\nb:2");
    runTestCase(2, "test2.txt", "");
    runTestCase(3, "test3.txt", "z:26");
    runTestCase(4, "test4.txt", "b:1\na:2\nb:3");
    runTestCase(5, "test5.txt", "key1:value1\n:onlyvalue\nkeyonly:\nmalformed\nkey2:value2");

    return 0;
}