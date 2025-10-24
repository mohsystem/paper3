
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>

using namespace std;

vector<pair<string, string>> readAndSortFile(const string& filename) {
    vector<pair<string, string>> records;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return records;
    }
    
    string line;
    while (getline(file, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \\t\\r\\n");
        size_t end = line.find_last_not_of(" \\t\\r\\n");
        
        if (start != string::npos && end != string::npos) {
            line = line.substr(start, end - start + 1);
        }
        
        if (!line.empty()) {
            size_t pos = line.find('=');
            if (pos != string::npos) {
                string key = line.substr(0, pos);
                string value = line.substr(pos + 1);
                
                // Trim key and value
                size_t keyStart = key.find_first_not_of(" \\t");
                size_t keyEnd = key.find_last_not_of(" \\t");
                if (keyStart != string::npos && keyEnd != string::npos) {
                    key = key.substr(keyStart, keyEnd - keyStart + 1);
                }
                
                size_t valStart = value.find_first_not_of(" \\t");
                size_t valEnd = value.find_last_not_of(" \\t");
                if (valStart != string::npos && valEnd != string::npos) {
                    value = value.substr(valStart, valEnd - valStart + 1);
                }
                
                records.push_back(make_pair(key, value));
            }
        }
    }
    
    file.close();
    
    // Sort by key
    sort(records.begin(), records.end(), 
         [](const pair<string, string>& a, const pair<string, string>& b) {
             return a.first < b.first;
         });
    
    return records;
}

void createTestFile(const string& filename, const vector<string>& lines) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error creating file: " << filename << endl;
        return;
    }
    
    for (const string& line : lines) {
        file << line << endl;
    }
    
    file.close();
}

void printRecords(const vector<pair<string, string>>& records) {
    for (const auto& record : records) {
        cout << record.first << " = " << record.second << endl;
    }
}

int main() {
    // Test case 1: Basic key-value pairs
    createTestFile("test1.txt", {"name=John", "age=30", "city=NewYork"});
    cout << "Test 1:" << endl;
    printRecords(readAndSortFile("test1.txt"));
    
    // Test case 2: Unsorted keys
    createTestFile("test2.txt", {"zebra=animal", "apple=fruit", "car=vehicle"});
    cout << "\\nTest 2:" << endl;
    printRecords(readAndSortFile("test2.txt"));
    
    // Test case 3: Keys with spaces
    createTestFile("test3.txt", {"first name=Alice", "last name=Smith", "age=25"});
    cout << "\\nTest 3:" << endl;
    printRecords(readAndSortFile("test3.txt"));
    
    // Test case 4: Duplicate keys
    createTestFile("test4.txt", {"color=red", "color=blue", "shape=circle"});
    cout << "\\nTest 4:" << endl;
    printRecords(readAndSortFile("test4.txt"));
    
    // Test case 5: Empty and invalid lines
    createTestFile("test5.txt", {"valid=data", "", "invalid_line", "another=valid"});
    cout << "\\nTest 5:" << endl;
    printRecords(readAndSortFile("test5.txt"));
    
    return 0;
}
