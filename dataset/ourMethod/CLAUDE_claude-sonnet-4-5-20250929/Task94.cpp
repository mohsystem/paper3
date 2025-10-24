
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <limits>

// Maximum file size to prevent excessive memory usage (10MB)
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
// Maximum number of records to prevent DoS
const size_t MAX_RECORDS = 100000;
// Maximum key/value length
const size_t MAX_KV_LENGTH = 1024;

struct KeyValue {
    std::string key;
    std::string value;
    
    // Comparator for sorting by key
    bool operator<(const KeyValue& other) const {
        return key < other.key;
    }
};

// Validates that path doesn't contain directory traversal attempts\n// Returns true if path is safe\nbool validatePath(const std::string& path) {\n    // Check for null or empty path\n    if (path.empty() || path.length() > 4096) {\n        return false;\n    }\n    \n    // Reject paths with directory traversal patterns\n    if (path.find("..") != std::string::npos) {\n        return false;\n    }\n    \n    // Reject absolute paths to enforce relative path usage\n    if (!path.empty() && path[0] == '/') {\n        return false;\n    }\n    \n    return true;\n}\n\n// Safely parse a line into key-value pair\n// Returns true if parsing successful\nbool parseLine(const std::string& line, KeyValue& kv) {\n    // Skip empty lines and validate length\n    if (line.empty() || line.length() > MAX_KV_LENGTH * 2) {\n        return false;\n    }\n    \n    // Find delimiter (expecting format: key=value or key:value)\n    size_t delimPos = line.find('=');\n    if (delimPos == std::string::npos) {\n        delimPos = line.find(':');
        if (delimPos == std::string::npos) {
            return false;
        }
    }
    
    // Extract key and value with bounds checking
    if (delimPos == 0 || delimPos >= line.length() - 1) {
        return false; // Empty key or value
    }
    
    std::string key = line.substr(0, delimPos);
    std::string value = line.substr(delimPos + 1);
    
    // Trim whitespace safely
    size_t keyStart = key.find_first_not_of(" \\t\\r\\n");
    size_t keyEnd = key.find_last_not_of(" \\t\\r\\n");
    size_t valStart = value.find_first_not_of(" \\t\\r\\n");
    size_t valEnd = value.find_last_not_of(" \\t\\r\\n");
    
    if (keyStart == std::string::npos || valStart == std::string::npos) {
        return false; // Empty after trim
    }
    
    kv.key = key.substr(keyStart, keyEnd - keyStart + 1);
    kv.value = value.substr(valStart, valEnd - valStart + 1);
    
    // Validate lengths after parsing
    if (kv.key.length() > MAX_KV_LENGTH || kv.value.length() > MAX_KV_LENGTH) {
        return false;
    }
    
    return true;
}

// Read and sort key-value records from file
// Returns sorted vector of records or empty vector on error
std::vector<KeyValue> readAndSortRecords(const std::string& filename) {
    std::vector<KeyValue> records;
    
    // Validate file path
    if (!validatePath(filename)) {
        std::cerr << "Error: Invalid file path" << std::endl;
        return records;
    }
    
    // Open file with input stream - no wx mode in C++ streams, but we validate carefully
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return records;
    }
    
    // Check file size to prevent excessive memory usage
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    if (fileSize < 0 || static_cast<size_t>(fileSize) > MAX_FILE_SIZE) {
        std::cerr << "Error: File too large or invalid" << std::endl;
        file.close();
        return records;
    }
    file.seekg(0, std::ios::beg);
    
    std::string line;
    size_t lineCount = 0;
    
    // Read file line by line with bounds checking
    while (std::getline(file, line) && lineCount < MAX_RECORDS) {
        lineCount++;
        
        // Validate line length before processing
        if (line.length() > MAX_KV_LENGTH * 2) {
            std::cerr << "Warning: Line " << lineCount << " exceeds maximum length, skipping" << std::endl;
            continue;
        }
        
        KeyValue kv;
        if (parseLine(line, kv)) {
            records.push_back(kv);
        }
    }
    
    file.close();
    
    if (lineCount >= MAX_RECORDS) {
        std::cerr << "Warning: Maximum record count reached" << std::endl;
    }
    
    // Sort records by key
    std::sort(records.begin(), records.end());
    
    return records;
}

int main() {
    // Test case 1: Basic key-value pairs
    {
        std::ofstream out("test1.txt");
        out << "name=John\\n";
        out << "age=30\\n";
        out << "city=NYC\\n";
        out.close();
        
        std::vector<KeyValue> result = readAndSortRecords("test1.txt");
        std::cout << "Test 1 - Basic sorting:" << std::endl;
        for (const auto& kv : result) {
            std::cout << kv.key << "=" << kv.value << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test case 2: Colon delimiter
    {
        std::ofstream out("test2.txt");
        out << "zebra:animal\\n";
        out << "apple:fruit\\n";
        out << "carrot:vegetable\\n";
        out.close();
        
        std::vector<KeyValue> result = readAndSortRecords("test2.txt");
        std::cout << "Test 2 - Colon delimiter:" << std::endl;
        for (const auto& kv : result) {
            std::cout << kv.key << ":" << kv.value << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test case 3: Whitespace handling
    {
        std::ofstream out("test3.txt");
        out << "  key1  =  value1  \\n";
        out << "key2=value2\\n";
        out << "\\n";
        out << "key0=value0\\n";
        out.close();
        
        std::vector<KeyValue> result = readAndSortRecords("test3.txt");
        std::cout << "Test 3 - Whitespace handling:" << std::endl;
        for (const auto& kv : result) {
            std::cout << kv.key << "=" << kv.value << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test case 4: Invalid file path
    {
        std::cout << "Test 4 - Invalid path (../etc/passwd):" << std::endl;
        std::vector<KeyValue> result = readAndSortRecords("../etc/passwd");
        std::cout << "Records read: " << result.size() << std::endl << std::endl;
    }
    
    // Test case 5: Empty file
    {
        std::ofstream out("test5.txt");
        out.close();
        
        std::vector<KeyValue> result = readAndSortRecords("test5.txt");
        std::cout << "Test 5 - Empty file:" << std::endl;
        std::cout << "Records read: " << result.size() << std::endl;
    }
    
    return 0;
}
