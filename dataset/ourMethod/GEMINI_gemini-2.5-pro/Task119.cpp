#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <filesystem>

/**
 * Parses a string in CSV format.
 * This implementation handles quoted fields containing delimiters and newlines,
 * and escaped quotes ("").
 *
 * @param content The string content in CSV format.
 * @return A vector of vectors of strings representing the parsed CSV data.
 */
std::vector<std::vector<std::string>> parseCsv(const std::string& content) {
    std::vector<std::vector<std::string>> records;
    if (content.empty()) return records;

    std::vector<std::string> currentRecord;
    std::stringstream currentField;
    bool inQuotes = false;

    for (size_t i = 0; i < content.length(); ++i) {
        char c = content[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < content.length() && content[i + 1] == '"') {
                    currentField << '"';
                    i++; // Skip the next quote
                } else {
                    inQuotes = false;
                }
            } else {
                currentField << c;
            }
        } else {
            switch (c) {
                case '"':
                    if (currentField.tellp() == 0) { // Check if field is empty
                         inQuotes = true;
                    } else {
                         currentField << c; // Treat as a normal character
                    }
                    break;
                case ',':
                    currentRecord.push_back(currentField.str());
                    currentField.str("");
                    currentField.clear();
                    break;
                case '\n':
                case '\r':
                    if (c == '\r' && i + 1 < content.length() && content[i+1] == '\n') {
                        i++; // Skip LF in CRLF
                    }
                    currentRecord.push_back(currentField.str());
                    records.push_back(currentRecord);
                    currentRecord.clear();
                    currentField.str("");
                    currentField.clear();
                    break;
                default:
                    currentField << c;
                    break;
            }
        }
    }

    // Add the last record if the file doesn't end with a newline
    if (!currentRecord.empty() || currentField.tellp() > 0) {
        currentRecord.push_back(currentField.str());
        records.push_back(currentRecord);
    }

    return records;
}

/**
 * Reads and parses a CSV file from a given path.
 * @param filePath The path to the CSV file.
 * @return Parsed CSV data.
 * @throws std::runtime_error on file errors or security violations.
 */
std::vector<std::vector<std::string>> readAndParseCsv(const std::string& filePath) {
    if (filePath.find("..") != std::string::npos) {
        throw std::runtime_error("Path traversal attempt detected.");
    }

    std::filesystem::path path_obj(filePath);
    std::filesystem::path canonical_path;
    try {
       canonical_path = std::filesystem::canonical(path_obj);
    } catch(const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Error resolving path: " + std::string(e.what()));
    }
    
    std::filesystem::path base_dir = std::filesystem::current_path();
    // Poor man's starts_with for paths
    if (std::distance(base_dir.begin(), base_dir.end()) > std::distance(canonical_path.begin(), canonical_path.end()) ||
        !std::equal(base_dir.begin(), base_dir.end(), canonical_path.begin())) {
        throw std::runtime_error("File path is not within the allowed directory.");
    }
    
    if (!std::filesystem::is_regular_file(canonical_path)) {
        throw std::runtime_error("File does not exist or is not a regular file: " + filePath);
    }
    
    // RAII for file handling
    std::ifstream file(canonical_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return parseCsv(buffer.str());
}

void printRecords(const std::vector<std::vector<std::string>>& records) {
    for (const auto& row : records) {
        std::cout << "  [";
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
    std::vector<std::string> testCsvData = {
        "a,b,c\n1,2,3",
        "a,\"b,c\",d\n1,\"2,3\",4",
        "a,\"b\"\"c\",d\n1,\"2\"\"3\",4",
        "a,,c\n1,2,",
        "header1,header2\n\"field with\nnew line\",field2"
    };

    for (size_t i = 0; i < testCsvData.size(); ++i) {
        std::string filename = "test" + std::to_string(i + 1) + ".csv";
        try {
            { // Scoped to use RAII for file closing
                std::ofstream testFile(filename);
                if (!testFile) {
                    std::cerr << "Failed to create test file: " << filename << std::endl;
                    continue;
                }
                testFile << testCsvData[i];
            }

            std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
            std::cout << "Input CSV:\n" << testCsvData[i] << std::endl;
            
            auto result = readAndParseCsv(filename);
            
            std::cout << "Parsed Output:" << std::endl;
            printRecords(result);
            std::cout << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Test Case " << (i + 1) << " failed: " << e.what() << std::endl;
        }

        std::filesystem::remove(filename);
    }

    return 0;
}