#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**
 * @brief Parses a string in CSV format into a vector of vectors of strings.
 * This is a simple parser and does not handle quoted fields.
 * 
 * @param csvData The const reference to a string containing CSV data.
 * @return A 2D vector of strings representing the parsed CSV.
 */
std::vector<std::vector<std::string>> parseCSV(const std::string& csvData) {
    std::vector<std::vector<std::string>> records;
    if (csvData.empty()) {
        return records;
    }

    std::stringstream dataStream(csvData);
    std::string rowStr;

    // Read each line from the string stream
    while (std::getline(dataStream, rowStr)) {
        std::vector<std::string> row;
        std::stringstream rowStream(rowStr);
        std::string field;

        // Read each comma-separated field from the line stream
        while (std::getline(rowStream, field, ',')) {
            row.push_back(field);
        }
        // Handle trailing comma case (e.g., "a,b,")
        if (!rowStr.empty() && rowStr.back() == ',') {
            row.push_back("");
        }

        records.push_back(row);
    }

    return records;
}

void runTestCase(const std::string& testName, const std::string& input) {
    std::cout << "--- " << testName << " ---" << std::endl;
    std::cout << "Input:\n\"" << input << "\"" << std::endl;
    std::vector<std::vector<std::string>> parsedData = parseCSV(input);
    
    std::cout << "Output:" << std::endl;
    if (parsedData.empty()) {
        std::cout << "[]" << std::endl;
    } else {
        std::cout << "[" << std::endl;
        for (const auto& row : parsedData) {
            std::cout << "  [";
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << "\"" << row[i] << "\"";
                if (i < row.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "]" << std::endl;
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    runTestCase("Test Case 1: Standard CSV", "a,b,c\n1,2,3\nx,y,z");
    runTestCase("Test Case 2: Empty fields", "a,,c\n1,2,\n,y,z");
    runTestCase("Test Case 3: Single row", "one,two,three,four");
    runTestCase("Test Case 4: Single column", "one\ntwo\nthree");
    runTestCase("Test Case 5: Empty input", "");
    
    return 0;
}