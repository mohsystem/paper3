
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdio>
#include <sys/stat.h>

const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

std::vector<std::pair<std::string, std::string>> readAndSortKeyValueFile(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> records;
    
    try {
        // Check if file exists and get size
        struct stat fileStat;
        if (stat(filename.c_str(), &fileStat) != 0) {
            std::cerr << "File not found: " << filename << std::endl;
            return records;
        }
        
        // Check file size
        if (fileStat.st_size > MAX_FILE_SIZE) {
            std::cerr << "File size exceeds maximum allowed size" << std::endl;
            return records;
        }
        
        // Open file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return records;
        }
        
        std::string line;
        int lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \\t\\r\\n"));
            line.erase(line.find_last_not_of(" \\t\\r\\n") + 1);
            
            // Skip empty lines
            if (line.empty()) {
                continue;
            }
            
            // Find separator (= or :)
            size_t separatorPos = line.find('=');
            if (separatorPos == std::string::npos) {
                separatorPos = line.find(':');
            }
            
            if (separatorPos != std::string::npos && separatorPos > 0) {
                std::string key = line.substr(0, separatorPos);
                std::string value = line.substr(separatorPos + 1);
                
                // Trim key and value
                key.erase(0, key.find_first_not_of(" \\t"));
                key.erase(key.find_last_not_of(" \\t") + 1);
                value.erase(0, value.find_first_not_of(" \\t"));
                value.erase(value.find_last_not_of(" \\t") + 1);
                
                records.push_back(std::make_pair(key, value));
            } else {
                std::cerr << "Invalid format at line " << lineNumber << ": " << line << std::endl;
            }
        }
        
        file.close();
        
        // Sort by key
        std::sort(records.begin(), records.end(),
                  [](const std::pair<std::string, std::string>& a,
                     const std::pair<std::string, std::string>& b) {
                      return a.first < b.first;
                  });
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return records;
}

int main() {
    const char* testFile = "test_keyvalue.txt";
    
    // Test case 1: Basic key-value pairs
    {
        std::ofstream file(testFile);
        file << "name=John\\n";
        file << "age=30\\n";
        file << "city=NewYork\\n";
        file << "country=USA\\n";
        file << "email=john@example.com\\n";
        file.close();
        
        std::cout << "Test Case 1: Basic key-value pairs" << std::endl;
        auto result1 = readAndSortKeyValueFile(testFile);
        for (const auto& entry : result1) {
            std::cout << entry.first << " = " << entry.second << std::endl;
        }
    }
    
    // Test case 2: Colon separator
    {
        std::ofstream file(testFile);
        file << "zebra:animal\\n";
        file << "apple:fruit\\n";
        file << "carrot:vegetable\\n";
        file.close();
        
        std::cout << "\\nTest Case 2: Colon separator" << std::endl;
        auto result2 = readAndSortKeyValueFile(testFile);
        for (const auto& entry : result2) {
            std::cout << entry.first << " : " << entry.second << std::endl;
        }
    }
    
    // Test case 3: Mixed with empty lines
    {
        std::ofstream file(testFile);
        file << "dog=animal\\n";
        file << "\\n";
        file << "banana=fruit\\n";
        file << "apple=fruit\\n";
        file.close();
        
        std::cout << "\\nTest Case 3: With empty lines" << std::endl;
        auto result3 = readAndSortKeyValueFile(testFile);
        for (const auto& entry : result3) {
            std::cout << entry.first << " = " << entry.second << std::endl;
        }
    }
    
    // Test case 4: Non-existent file
    {
        std::cout << "\\nTest Case 4: Non-existent file" << std::endl;
        auto result4 = readAndSortKeyValueFile("nonexistent.txt");
        std::cout << "Records found: " << result4.size() << std::endl;
    }
    
    // Test case 5: Invalid format
    {
        std::ofstream file(testFile);
        file << "validkey=validvalue\\n";
        file << "invalidline\\n";
        file << "anotherkey=anothervalue\\n";
        file.close();
        
        std::cout << "\\nTest Case 5: Invalid format handling" << std::endl;
        auto result5 = readAndSortKeyValueFile(testFile);
        for (const auto& entry : result5) {
            std::cout << entry.first << " = " << entry.second << std::endl;
        }
    }
    
    // Cleanup
    std::remove(testFile);
    
    return 0;
}
