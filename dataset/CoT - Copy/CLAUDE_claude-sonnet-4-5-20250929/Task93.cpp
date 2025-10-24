
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <sys/stat.h>

const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

class KeyValuePair {
private:
    std::string key;
    std::string value;

public:
    KeyValuePair(const std::string& k, const std::string& v) 
        : key(k), value(v) {}
    
    std::string getKey() const {
        return key;
    }
    
    std::string getValue() const {
        return value;
    }
    
    bool operator<(const KeyValuePair& other) const {
        return key < other.key;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const KeyValuePair& pair) {
        os << pair.key << "=" << pair.value;
        return os;
    }
};

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

long getFileSize(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

std::vector<KeyValuePair> readAndSortKeyValueFile(const std::string& filePath) {
    std::vector<KeyValuePair> records;
    
    if (filePath.empty()) {
        std::cerr << "Invalid file path" << std::endl;
        return records;
    }
    
    long fileSize = getFileSize(filePath);
    if (fileSize < 0) {
        std::cerr << "Error: Cannot access file - " << filePath << std::endl;
        return records;
    }
    
    if (fileSize > MAX_FILE_SIZE) {
        std::cerr << "File size exceeds maximum allowed size" << std::endl;
        return records;
    }
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file - " << filePath << std::endl;
        return records;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse key-value pair
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            records.push_back(KeyValuePair(key, value));
        } else {
            std::cerr << "Malformed line " << lineNumber << ": " << line << std::endl;
        }
    }
    
    file.close();
    
    // Sort records by key
    std::sort(records.begin(), records.end());
    
    return records;
}

void writeTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << std::endl;
    }
    file.close();
}

int main() {
    // Test case 1: Create and read a valid file
    std::cout << "Test 1 - Valid file:" << std::endl;
    std::string testFile1 = "test1.txt";
    writeTestFile(testFile1, {
        "name=John",
        "age=30",
        "city=NewYork",
        "country=USA"
    });
    
    std::vector<KeyValuePair> result1 = readAndSortKeyValueFile(testFile1);
    for (const auto& pair : result1) {
        std::cout << pair << std::endl;
    }
    remove(testFile1.c_str());
    std::cout << std::endl;
    
    // Test case 2: File with comments and empty lines
    std::cout << "Test 2 - File with comments:" << std::endl;
    std::string testFile2 = "test2.txt";
    writeTestFile(testFile2, {
        "# This is a comment",
        "zebra=animal",
        "",
        "apple=fruit",
        "banana=fruit"
    });
    
    std::vector<KeyValuePair> result2 = readAndSortKeyValueFile(testFile2);
    for (const auto& pair : result2) {
        std::cout << pair << std::endl;
    }
    remove(testFile2.c_str());
    std::cout << std::endl;
    
    // Test case 3: File with malformed lines
    std::cout << "Test 3 - Malformed lines:" << std::endl;
    std::string testFile3 = "test3.txt";
    writeTestFile(testFile3, {
        "valid=line",
        "invalid line without equals",
        "another=valid"
    });
    
    std::vector<KeyValuePair> result3 = readAndSortKeyValueFile(testFile3);
    for (const auto& pair : result3) {
        std::cout << pair << std::endl;
    }
    remove(testFile3.c_str());
    std::cout << std::endl;
    
    // Test case 4: Non-existent file
    std::cout << "Test 4 - Non-existent file:" << std::endl;
    std::vector<KeyValuePair> result4 = readAndSortKeyValueFile("nonexistent.txt");
    std::cout << "Records found: " << result4.size() << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Empty file path
    std::cout << "Test 5 - Empty file path:" << std::endl;
    std::vector<KeyValuePair> result5 = readAndSortKeyValueFile("");
    std::cout << "Records found: " << result5.size() << std::endl;
    
    return 0;
}
