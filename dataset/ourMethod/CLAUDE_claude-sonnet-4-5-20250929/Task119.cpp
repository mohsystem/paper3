
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <limits>

// Maximum file size: 100MB to prevent excessive memory usage
const size_t MAX_FILE_SIZE = 100 * 1024 * 1024;
// Maximum field size: 1MB to prevent memory attacks
const size_t MAX_FIELD_SIZE = 1 * 1024 * 1024;
// Maximum number of fields per row
const size_t MAX_FIELDS_PER_ROW = 10000;

class CSVParser {
private:
    std::string base_directory;
    
    // Normalize and validate path to prevent directory traversal (CWE-22)
    bool is_safe_path(const std::string& filepath, std::string& normalized_path) {
        // Reject empty paths
        if (filepath.empty()) {
            return false;
        }
        
        // Reject paths with null bytes (CWE-158)
        if (filepath.find('\\0') != std::string::npos) {
            return false;
        }
        
        // Reject dangerous patterns that could lead to directory traversal
        if (filepath.find("..") != std::string::npos ||
            filepath.find("//") != std::string::npos ||
            filepath[0] == '/' ||
            (filepath.length() > 1 && filepath[1] == ':')) {  // Windows drive letter
            return false;
        }
        
        // Construct full path within base directory
        normalized_path = base_directory.empty() ? filepath : base_directory + "/" + filepath;
        
        // Additional validation: ensure no backslashes (Windows path separator abuse)
        if (normalized_path.find('\\\\') != std::string::npos) {\n            return false;\n        }\n        \n        return true;\n    }\n    \n    // Parse a single CSV field handling quoted fields and escapes\n    bool parse_field(const std::string& line, size_t& pos, std::string& field) {\n        field.clear();\n        \n        if (pos >= line.length()) {\n            return true;  // End of line\n        }\n        \n        bool in_quotes = false;\n        \n        // Check if field starts with quote\n        if (line[pos] == '"') {\n            in_quotes = true;\n            pos++;  // Skip opening quote\n        }\n        \n        while (pos < line.length()) {\n            // Prevent excessively large fields (CWE-400: Resource exhaustion)\n            if (field.size() >= MAX_FIELD_SIZE) {\n                return false;\n            }\n            \n            char current = line[pos];\n            \n            if (in_quotes) {\n                if (current == '"') {\n                    // Check for escaped quote (double quote)\n                    if (pos + 1 < line.length() && line[pos + 1] == '"') {\n                        field += '"';\n                        pos += 2;\n                    } else {\n                        // End of quoted field\n                        in_quotes = false;\n                        pos++;\n                        // Skip to next comma or end\n                        if (pos < line.length() && line[pos] == ',') {\n                            pos++;\n                        }\n                        break;\n                    }\n                } else {\n                    field += current;\n                    pos++;\n                }\n            } else {\n                if (current == ',') {\n                    pos++;  // Skip comma\n                    break;\n                } else {\n                    field += current;\n                    pos++;\n                }\n            }\n        }\n        \n        return true;\n    }\n    \npublic:\n    CSVParser(const std::string& base_dir = ".") : base_directory(base_dir) {}\n    \n    // Parse CSV file and return rows as vector of vectors\n    // Returns empty vector on error\n    std::vector<std::vector<std::string>> parse_file(const std::string& filepath) {\n        std::vector<std::vector<std::string>> result;\n        \n        // Validate and normalize path (CWE-22: Path traversal prevention)\n        std::string safe_path;\n        if (!is_safe_path(filepath, safe_path)) {\n            std::cerr << "Error: Invalid or unsafe file path" << std::endl;\n            return result;\n        }\n        \n        // Open file with validation (CWE-73: External control of file name)\n        // Use binary mode to handle different line endings properly\n        std::ifstream file(safe_path, std::ios::binary | std::ios::ate);\n        if (!file.is_open()) {\n            std::cerr << "Error: Cannot open file: " << safe_path << std::endl;\n            return result;\n        }\n        \n        // Check file size to prevent resource exhaustion (CWE-400)\n        std::streamsize file_size = file.tellg();\n        if (file_size < 0 || static_cast<size_t>(file_size) > MAX_FILE_SIZE) {\n            std::cerr << "Error: File size invalid or exceeds maximum allowed size" << std::endl;\n            return result;\n        }\n        \n        file.seekg(0, std::ios::beg);\n        \n        std::string line;\n        size_t line_number = 0;\n        \n        // Read and parse each line\n        while (std::getline(file, line)) {\n            line_number++;\n            \n            // Remove carriage return if present (handle Windows line endings)\n            if (!line.empty() && line.back() == '\\r') {\n                line.pop_back();\n            }\n            \n            // Skip empty lines\n            if (line.empty()) {\n                continue;\n            }\n            \n            std::vector<std::string> row;\n            size_t pos = 0;\n            \n            // Parse all fields in the line\n            while (pos <= line.length()) {\n                // Prevent too many fields (CWE-400: Resource exhaustion)\n                if (row.size() >= MAX_FIELDS_PER_ROW) {\n                    std::cerr << "Error: Too many fields in line " << line_number << std::endl;\n                    return std::vector<std::vector<std::string>>();\n                }\n                \n                std::string field;\n                if (!parse_field(line, pos, field)) {\n                    std::cerr << "Error: Field too large at line " << line_number << std::endl;\n                    return std::vector<std::vector<std::string>>();\n                }\n                \n                row.push_back(field);\n                \n                // Break if we've reached end of line
                if (pos >= line.length()) {
                    break;
                }
            }
            
            if (!row.empty()) {
                result.push_back(row);
            }
        }
        
        file.close();
        return result;
    }
    
    // Write CSV data to file safely
    bool write_file(const std::string& filepath, const std::vector<std::vector<std::string>>& data) {
        // Validate and normalize path (CWE-22: Path traversal prevention)
        std::string safe_path;
        if (!is_safe_path(filepath, safe_path)) {
            std::cerr << "Error: Invalid or unsafe file path" << std::endl;
            return false;
        }
        
        // Write to temporary file first to avoid TOCTOU issues (CWE-367)
        std::string temp_path = safe_path + ".tmp";
        
        std::ofstream file(temp_path, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot create temporary file" << std::endl;
            return false;
        }
        
        // Write CSV data with proper escaping
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); i++) {
                const std::string& field = row[i];
                
                // Check if field needs quoting (contains comma, quote, or newline)
                bool needs_quotes = field.find(',') != std::string::npos ||
                                   field.find('"') != std::string::npos ||
                                   field.find('\\n') != std::string::npos ||
                                   field.find('\\r') != std::string::npos;
                
                if (needs_quotes) {
                    file << '"';
                    // Escape quotes by doubling them
                    for (char c : field) {
                        if (c == '"') {
                            file << "\\"\\"";
                        } else {
                            file << c;
                        }
                    }
                    file << '"';
                } else {
                    file << field;
                }
                
                if (i < row.size() - 1) {
                    file << ',';
                }
            }
            file << '\\n';
        }
        
        // Ensure data is written to disk (prevent data loss)
        file.flush();
        file.close();
        
        // Atomic rename to prevent TOCTOU (CWE-367)
        if (std::rename(temp_path.c_str(), safe_path.c_str()) != 0) {
            std::cerr << "Error: Failed to rename temporary file" << std::endl;
            std::remove(temp_path.c_str());  // Clean up temp file
            return false;
        }
        
        return true;
    }
};

int main() {
    CSVParser parser(".");
    
    // Test case 1: Create and write a simple CSV file
    std::cout << "Test 1: Writing simple CSV file" << std::endl;
    std::vector<std::vector<std::string>> test_data1 = {
        {"Name", "Age", "City"},
        {"Alice", "30", "New York"},
        {"Bob", "25", "Los Angeles"}
    };
    
    if (parser.write_file("test1.csv", test_data1)) {
        std::cout << "Successfully wrote test1.csv" << std::endl;
        auto result = parser.parse_file("test1.csv");
        std::cout << "Parsed " << result.size() << " rows" << std::endl;
    }
    
    // Test case 2: CSV with quoted fields containing commas
    std::cout << "\\nTest 2: CSV with special characters" << std::endl;
    std::vector<std::vector<std::string>> test_data2 = {
        {"Product", "Description", "Price"},
        {"Widget", "A small, useful item", "10.99"},
        {"Gadget", "Contains \\"special\\" features", "25.50"}
    };
    
    if (parser.write_file("test2.csv", test_data2)) {
        std::cout << "Successfully wrote test2.csv" << std::endl;
        auto result = parser.parse_file("test2.csv");
        for (const auto& row : result) {
            for (size_t i = 0; i < row.size(); i++) {
                std::cout << "[" << row[i] << "]";
                if (i < row.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }
    
    // Test case 3: Empty CSV
    std::cout << "\\nTest 3: Empty CSV" << std::endl;
    std::vector<std::vector<std::string>> test_data3;
    if (parser.write_file("test3.csv", test_data3)) {
        std::cout << "Successfully wrote test3.csv" << std::endl;
        auto result = parser.parse_file("test3.csv");
        std::cout << "Parsed " << result.size() << " rows" << std::endl;
    }
    
    // Test case 4: Path traversal attempt (should fail)
    std::cout << "\\nTest 4: Path traversal attempt (should fail)" << std::endl;
    auto result = parser.parse_file("../etc/passwd");
    if (result.empty()) {
        std::cout << "Correctly rejected unsafe path" << std::endl;
    }
    
    // Test case 5: CSV with newlines in quoted fields
    std::cout << "\\nTest 5: CSV with embedded newlines" << std::endl;
    std::vector<std::vector<std::string>> test_data5 = {
        {"Field1", "Field2"},
        {"Value1", "Line1\\nLine2\\nLine3"},
        {"Value2", "Normal field"}
    };
    
    if (parser.write_file("test5.csv", test_data5)) {
        std::cout << "Successfully wrote test5.csv" << std::endl;
        auto result = parser.parse_file("test5.csv");
        std::cout << "Parsed " << result.size() << " rows" << std::endl;
    }
    
    return 0;
}
