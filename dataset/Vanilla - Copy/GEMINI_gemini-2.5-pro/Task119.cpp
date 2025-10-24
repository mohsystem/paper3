#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**
 * Parses a CSV string into a vector of vectors of strings.
 * This implementation handles quoted fields containing commas and escaped double quotes.
 *
 * @param csvContent The string content of the CSV file.
 * @return A vector of vectors of strings representing the parsed CSV data.
 */
std::vector<std::vector<std::string>> parseCsv(const std::string& csvContent) {
    std::vector<std::vector<std::string>> records;
    if (csvContent.empty()) {
        return records;
    }

    std::stringstream ss(csvContent);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        std::vector<std::string> currentRecord;
        std::string currentField;
        bool inQuotes = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];

            if (inQuotes) {
                if (c == '"') {
                    // Check for escaped quote ("")
                    if (i + 1 < line.length() && line[i + 1] == '"') {
                        currentField += '"';
                        i++; // Skip the next quote
                    } else {
                        inQuotes = false;
                    }
                } else {
                    currentField += c;
                }
            } else {
                if (c == '"') {
                    inQuotes = true;
                } else if (c == ',') {
                    currentRecord.push_back(currentField);
                    currentField.clear();
                } else {
                    currentField += c;
                }
            }
        }
        currentRecord.push_back(currentField);
        records.push_back(currentRecord);
    }
    return records;
}

void printRecords(const std::vector<std::vector<std::string>>& records) {
    for (const auto& row : records) {
        std::cout << "[";
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << "\"" << row[i] << "\"";
            if (i < row.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

int main() {
    std::vector<std::string> testCases = {
        // Test Case 1: Simple CSV
        "a,b,c\n1,2,3",
        // Test Case 2: Quoted fields with commas
        "\"a,b\",c\n1,\"2,3\"",
        // Test Case 3: Empty fields and empty quoted fields
        "a,,c\n1,\"\",3",
        // Test Case 4: Escaped quotes within a quoted field
        "a,\"b\"\"c\",d\n\"e \"\"f\"\"\",g",
        // Test Case 5: Mixed and complex cases
        "Name,Age,\"Address, City\"\nJohn Doe,30,\"123 Main St, \"\"Anytown\"\"\"\n\"Jane, Smith\",25,\"456 Oak Ave, Somewhere\""
    };

    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input:\n" << testCases[i] << std::endl;
        std::vector<std::vector<std::string>> result = parseCsv(testCases[i]);
        std::cout << "Output:" << std::endl;
        printRecords(result);
        std::cout << std::endl;
    }

    return 0;
}