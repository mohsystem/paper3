
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <limits>

// Structure to hold key-value pairs
struct KeyValue {
    std::string key;
    std::string value;
    
    // Comparison operator for sorting
    bool operator<(const KeyValue& other) const {
        return key < other.key;
    }
};

// Sanitize path to prevent directory traversal
bool isPathSafe(const std::string& filepath, const std::string& baseDir) {
    // Reject empty paths
    if (filepath.empty()) {
        return false;
    }
    
    // Reject paths with parent directory references or absolute paths
    if (filepath.find("..") != std::string::npos || 
        filepath[0] == '/' || 
        (filepath.length() > 1 && filepath[1] == ':')) {
        return false;
    }
    
    return true;
}

// Parse a single line into key-value pair
// Expected format: key=value or key:value
bool parseLine(const std::string& line, KeyValue& kv) {
    // Skip empty lines and lines starting with # (comments)
    if (line.empty() || line[0] == '#') {
        return false;
    }
    
    // Find delimiter (= or :)
    size_t delimPos = line.find('=');
    if (delimPos == std::string::npos) {
        delimPos = line.find(':');
    }
    
    if (delimPos == std::string::npos || delimPos == 0) {
        return false; // Invalid format
    }
    
    // Extract key and value with bounds checking
    kv.key = line.substr(0, delimPos);
    if (delimPos + 1 < line.length()) {
        kv.value = line.substr(delimPos + 1);
    } else {
        kv.value = "";
    }
    
    // Trim whitespace from key and value
    kv.key.erase(0, kv.key.find_first_not_of(" \\t\\r\\n"));
    kv.key.erase(kv.key.find_last_not_of(" \\t\\r\\n") + 1);
    kv.value.erase(0, kv.value.find_first_not_of(" \\t\\r\\n"));
    kv.value.erase(kv.value.find_last_not_of(" \\t\\r\\n") + 1);
    
    return !kv.key.empty();
}

// Read and sort key-value pairs from file
std::vector<KeyValue> readAndSortFile(const std::string& filepath) {
    std::vector<KeyValue> records;
    const size_t MAX_RECORDS = 100000; // Prevent excessive memory usage
    const size_t MAX_LINE_LENGTH = 4096; // Prevent reading extremely long lines
    
    // Validate path safety
    if (!isPathSafe(filepath, ".")) {
        std::cerr << "Error: Invalid file path" << std::endl;
        return records;
    }
    
    // Open file with RAII (ifstream automatically closes on destruction)
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filepath << std::endl;
        return records;
    }
    
    std::string line;
    line.reserve(256); // Pre-allocate reasonable size
    
    // Read file line by line
    while (std::getline(file, line)) {
        // Bounds check: prevent excessive line length
        if (line.length() > MAX_LINE_LENGTH) {
            std::cerr << "Warning: Line too long, skipping" << std::endl;
            continue;
        }
        
        // Bounds check: prevent excessive memory usage
        if (records.size() >= MAX_RECORDS) {
            std::cerr << "Warning: Maximum record limit reached" << std::endl;
            break;
        }
        
        KeyValue kv;
        if (parseLine(line, kv)) {
            records.push_back(kv);
        }
    }
    
    // Check for read errors
    if (file.bad()) {
        std::cerr << "Error: Failed to read file" << std::endl;
        records.clear();
        return records;
    }
    
    // Sort records by key
    std::sort(records.begin(), records.end());
    
    return records;
}

int main() {
    // Test case 1: Basic key-value pairs
    {
        std::ofstream testFile("test1.txt");
        if (testFile.is_open()) {
            testFile << "name=John\\n";
            testFile << "age=30\\n";
            testFile << "city=NewYork\\n";
            testFile.close();
            
            std::cout << "Test 1: Basic key-value pairs\\n";
            std::vector<KeyValue> result = readAndSortFile("test1.txt");
            for (const auto& kv : result) {
                std::cout << kv.key << " = " << kv.value << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    // Test case 2: Colon delimiter
    {
        std::ofstream testFile("test2.txt");
        if (testFile.is_open()) {
            testFile << "zebra:animal\\n";
            testFile << "apple:fruit\\n";
            testFile << "carrot:vegetable\\n";
            testFile.close();
            
            std::cout << "Test 2: Colon delimiter\\n";
            std::vector<KeyValue> result = readAndSortFile("test2.txt");
            for (const auto& kv : result) {
                std::cout << kv.key << " : " << kv.value << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    // Test case 3: Mixed with comments and empty lines
    {
        std::ofstream testFile("test3.txt");
        if (testFile.is_open()) {
            testFile << "# Configuration file\\n";
            testFile << "server=localhost\\n";
            testFile << "\\n";
            testFile << "port=8080\\n";
            testFile << "# End\\n";
            testFile.close();
            
            std::cout << "Test 3: With comments and empty lines\\n";
            std::vector<KeyValue> result = readAndSortFile("test3.txt");
            for (const auto& kv : result) {
                std::cout << kv.key << " = " << kv.value << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    // Test case 4: Whitespace handling
    {
        std::ofstream testFile("test4.txt");
        if (testFile.is_open()) {
            testFile << "  key1  =  value1  \\n";
            testFile << "key2=value2\\n";
            testFile << "  key3  :  value3  \\n";
            testFile.close();
            
            std::cout << "Test 4: Whitespace handling\\n";
            std::vector<KeyValue> result = readAndSortFile("test4.txt");
            for (const auto& kv : result) {
                std::cout << "[" << kv.key << "] = [" << kv.value << "]" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    // Test case 5: Invalid file path (security test)
    {
        std::cout << "Test 5: Invalid file path\\n";
        std::vector<KeyValue> result = readAndSortFile("../etc/passwd");
        std::cout << "Records read: " << result.size() << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
