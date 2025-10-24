#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**
 * Securely parses a CSV string into a vector of vectors of strings.
 * This implementation handles quoted fields containing commas and escaped quotes ("").
 * It processes the input using a stringstream to read line by line, preventing
 * issues with very large inputs if adapted for file streams. It avoids common
 * pitfalls like buffer overflows by using std::string and std::vector for
 * automatic memory management.
 *
 * @param csvContent The constant string reference containing the CSV data.
 * @return A vector of vectors of strings representing the parsed CSV.
 */
std::vector<std::vector<std::string>> parseCSV(const std::string& csvContent) {
    std::vector<std::vector<std::string>> records;
    if (csvContent.empty()) {
        return records;
    }

    std::stringstream contentStream(csvContent);
    std::string line;

    while (std::getline(contentStream, line)) {
        if (line.empty()) {
            records.push_back({}); // Add an empty row for an empty line
            continue;
        }

        std::vector<std::string> currentRow;
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
                    currentRow.push_back(currentField);
                    currentField.clear();
                } else {
                    currentField += c;
                }
            }
        }
        currentRow.push_back(currentField);
        records.push_back(currentRow);
    }

    return records;
}

void printResult(const std::vector<std::vector<std::string>>& data) {
    int rowNum = 0;
    for (const auto& row : data) {
        std::cout << "Row " << rowNum++ << ": [";
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
        "a,\"b,c\",d\n1,\"hello, world\",3",
        
        // Test Case 3: Escaped quotes inside quoted fields
        "a,\"b\"\"c\",d\n1,\"it's \"\"ok\"\"\",3",
        
        // Test Case 4: Malformed/edge cases (inconsistent columns, empty lines)
        "a,b\n1,2,3\n\n,d",
        
        // Test Case 5: Complex case with mixed quoting and commas
        "Name,Address,Notes\n\"Doe, John\",\"120 Main St, \"\"Anytown\"\"\",\"He said, \"\"Hello!\"\"\""
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input:\n" << testCases[i] << std::endl;
        std::vector<std::vector<std::string>> result = parseCSV(testCases[i]);
        std::cout << "Output:" << std::endl;
        printResult(result);
        std::cout << std::endl;
    }

    return 0;
}