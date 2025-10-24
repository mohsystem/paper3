#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility> // for std::pair
#include <cstdio>  // for remove()

// The function to read, parse, and sort records from a file
std::vector<std::pair<std::string, std::string>> sortFileRecords(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> records;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return records; // Return empty vector
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            records.push_back({key, value});
        }
    }

    file.close();

    // std::sort on a vector of pairs sorts by the first element by default
    std::sort(records.begin(), records.end());

    return records;
}

// Helper to create a file for testing
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }
}

int main() {
    std::vector<std::string> testContents = {
        "banana:fruit\napple:fruit\ncarrot:vegetable",
        "alpha:1\nbeta:2\ngamma:3",
        "zulu:end\nyankee:middle\nxray:start",
        "",
        "one:1"
    };

    for (size_t i = 0; i < testContents.size(); ++i) {
        std::string testFilename = "test_file_cpp_" + std::to_string(i) + ".txt";
        createTestFile(testFilename, testContents[i]);

        std::cout << "--- Test Case " << (i + 1) << " (C++) ---" << std::endl;
        std::cout << "Input from " << testFilename << ":" << std::endl;
        std::cout << (testContents[i].empty() ? "<empty>" : testContents[i]) << std::endl;

        std::vector<std::pair<std::string, std::string>> sortedRecords = sortFileRecords(testFilename);

        std::cout << "\nSorted Output:" << std::endl;
        if (!sortedRecords.empty()) {
            for (const auto& record : sortedRecords) {
                std::cout << record.first << ":" << record.second << std::endl;
            }
        } else {
            std::cout << "<no records to sort>" << std::endl;
        }
        std::cout << std::endl;

        remove(testFilename.c_str());
    }

    return 0;
}