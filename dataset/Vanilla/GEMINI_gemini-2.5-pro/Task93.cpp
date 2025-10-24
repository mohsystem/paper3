#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility> 
#include <cstdio>  

// Function to trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// Function to read, parse, and sort key-value records from a file
std::vector<std::pair<std::string, std::string>> sortFileRecords(const std::string& filePath) {
    std::vector<std::pair<std::string, std::string>> records;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return records;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos != std::string::npos) {
            std::string key = trim(line.substr(0, delimiterPos));
            std::string value = trim(line.substr(delimiterPos + 1));
            records.push_back({key, value});
        }
    }
    file.close();

    // Sort the vector of pairs based on the key (the first element)
    std::sort(records.begin(), records.end(), 
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    return records;
}

void runTestCase(int testNum, const std::string& fileName, const std::string& content) {
    std::cout << "--- Test Case " << testNum << " (" << fileName << ") ---" << std::endl;

    // Create test file
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create test file." << std::endl;
        return;
    }

    // Process file
    auto sortedRecords = sortFileRecords(fileName);

    // Print results
    if (sortedRecords.empty()) {
        std::cout << "No records found or file is empty." << std::endl;
    } else {
        for (const auto& record : sortedRecords) {
            std::cout << record.first << ":" << record.second << std::endl;
        }
    }

    // Clean up
    std::remove(fileName.c_str());
    std::cout << std::endl;
}

int main() {
    runTestCase(1, "test1.txt", "banana:fruit\napple:fruit\n carrot : vegetable ");
    runTestCase(2, "test2.txt", "zeta:26\nalpha:1\nbeta:2");
    runTestCase(3, "test3.txt", "");
    runTestCase(4, "test4.txt", "10:ten\n2:two\n1:one");
    runTestCase(5, "test5.txt", "apple:red\nApple:green\napple:yellow");
    return 0;
}