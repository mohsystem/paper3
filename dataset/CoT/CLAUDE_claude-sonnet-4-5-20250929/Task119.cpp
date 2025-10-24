
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstring>

#define MAX_FILE_SIZE (10 * 1024 * 1024)  // 10MB limit
#define MAX_ROWS 100000

class Task119 {
private:
    static std::string sanitizeField(const std::string& field) {
        if (field.empty()) {
            return "";
        }
        
        std::string sanitized = field;
        
        // Remove leading/trailing whitespace
        size_t start = sanitized.find_first_not_of(" \\t\\r\\n");
        size_t end = sanitized.find_last_not_of(" \\t\\r\\n");
        
        if (start == std::string::npos) {
            return "";
        }
        
        sanitized = sanitized.substr(start, end - start + 1);
        
        // Security: Prevent formula injection in CSV
        if (!sanitized.empty() && 
            (sanitized[0] == '=' || sanitized[0] == '+' || 
             sanitized[0] == '-' || sanitized[0] == '@')) {
            sanitized = "'" + sanitized;
        }
        
        return sanitized;
    }
    
    static std::string escapeField(const std::string& field) {
        if (field.empty()) {
            return "";
        }
        
        bool needsQuoting = false;
        std::string escaped = field;
        
        // Check if field needs quoting
        if (field.find(',') != std::string::npos || 
            field.find('"') != std::string::npos || 
            field.find('\\n') != std::string::npos) {
            needsQuoting = true;
        }
        
        // Escape quotes
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\"\\"");
            pos += 2;
        }
        
        if (needsQuoting) {
            escaped = "\\"" + escaped + "\\"";
        }
        
        return escaped;
    }
    
    static std::vector<std::string> parseLine(const std::string& line) {
        std::vector<std::string> fields;
        
        if (line.empty()) {
            return fields;
        }
        
        bool inQuotes = false;
        std::string currentField;
        
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            
            if (c == '"') {
                if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                    currentField += '"';
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.push_back(sanitizeField(currentField));
                currentField.clear();
            } else {
                currentField += c;
            }
        }
        
        fields.push_back(sanitizeField(currentField));
        return fields;
    }
    
    static bool isValidFilePath(const std::string& filePath) {
        // Security: Basic path traversal prevention
        if (filePath.find("..") != std::string::npos) {
            return false;
        }
        
        if (filePath.find("~") != std::string::npos) {
            return false;
        }
        
        return true;
    }

public:
    static std::vector<std::vector<std::string>> parseCSV(const std::string& filePath) {
        std::vector<std::vector<std::string>> records;
        
        if (filePath.empty()) {
            throw std::invalid_argument("File path cannot be empty");
        }
        
        // Security: Validate file path
        if (!isValidFilePath(filePath)) {
            throw std::runtime_error("Access denied: Invalid file path");
        }
        
        // Security: Validate file extension
        if (filePath.length() < 4 || 
            filePath.substr(filePath.length() - 4) != ".csv") {
            throw std::invalid_argument("Only CSV files are allowed");
        }
        
        std::ifstream file(filePath);
        
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }
        
        // Security: Check file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (fileSize > MAX_FILE_SIZE) {
            file.close();
            throw std::invalid_argument("File size exceeds maximum allowed size");
        }
        
        std::string line;
        int rowCount = 0;
        
        while (std::getline(file, line)) {
            // Security: Limit number of rows
            if (++rowCount > MAX_ROWS) {
                file.close();
                throw std::invalid_argument("File exceeds maximum row limit");
            }
            
            std::vector<std::string> record = parseLine(line);
            records.push_back(record);
        }
        
        file.close();
        return records;
    }
    
    static void writeCSV(const std::string& filePath, 
                        const std::vector<std::vector<std::string>>& data) {
        if (filePath.empty()) {
            throw std::invalid_argument("File path cannot be empty");
        }
        
        if (data.empty()) {
            throw std::invalid_argument("Data cannot be empty");
        }
        
        // Security: Validate file path
        if (!isValidFilePath(filePath)) {
            throw std::runtime_error("Access denied: Invalid file path");
        }
        
        std::ofstream file(filePath);
        
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing: " + filePath);
        }
        
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); i++) {
                file << escapeField(row[i]);
                if (i < row.size() - 1) {
                    file << ",";
                }
            }
            file << "\\n";
        }
        
        file.close();
    }
    
    static void printRecords(const std::vector<std::vector<std::string>>& records) {
        for (const auto& record : records) {
            std::cout << "[";
            for (size_t i = 0; i < record.size(); i++) {
                std::cout << record[i];
                if (i < record.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "]" << std::endl;
        }
    }
};

int main() {
    try {
        // Test case 1: Create and parse simple CSV
        std::vector<std::vector<std::string>> testData1 = {
            {"Name", "Age", "City"},
            {"John Doe", "30", "New York"},
            {"Jane Smith", "25", "Los Angeles"}
        };
        Task119::writeCSV("test1.csv", testData1);
        auto result1 = Task119::parseCSV("test1.csv");
        std::cout << "Test 1 - Simple CSV:" << std::endl;
        Task119::printRecords(result1);
        
        // Test case 2: CSV with quoted fields
        std::vector<std::vector<std::string>> testData2 = {
            {"Product", "Description", "Price"},
            {"Book", "A great book, highly recommended", "19.99"},
            {"Pen", "Blue ink pen", "2.50"}
        };
        Task119::writeCSV("test2.csv", testData2);
        auto result2 = Task119::parseCSV("test2.csv");
        std::cout << "\\nTest 2 - CSV with commas:" << std::endl;
        Task119::printRecords(result2);
        
        // Test case 3: CSV with special characters
        std::vector<std::vector<std::string>> testData3 = {
            {"ID", "Email", "Status"},
            {"001", "user@example.com", "Active"},
            {"002", "admin@test.org", "Inactive"}
        };
        Task119::writeCSV("test3.csv", testData3);
        auto result3 = Task119::parseCSV("test3.csv");
        std::cout << "\\nTest 3 - CSV with special characters:" << std::endl;
        Task119::printRecords(result3);
        
        // Test case 4: CSV with empty fields
        std::vector<std::vector<std::string>> testData4 = {
            {"Field1", "Field2", "Field3"},
            {"Value1", "", "Value3"},
            {"", "Value2", ""}
        };
        Task119::writeCSV("test4.csv", testData4);
        auto result4 = Task119::parseCSV("test4.csv");
        std::cout << "\\nTest 4 - CSV with empty fields:" << std::endl;
        Task119::printRecords(result4);
        
        // Test case 5: CSV with numeric data
        std::vector<std::vector<std::string>> testData5 = {
            {"Year", "Revenue", "Profit"},
            {"2021", "1000000", "250000"},
            {"2022", "1500000", "400000"}
        };
        Task119::writeCSV("test5.csv", testData5);
        auto result5 = Task119::parseCSV("test5.csv");
        std::cout << "\\nTest 5 - CSV with numeric data:" << std::endl;
        Task119::printRecords(result5);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
