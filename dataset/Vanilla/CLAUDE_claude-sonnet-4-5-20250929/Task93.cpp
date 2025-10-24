
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

vector<string> readAndSortFile(const string& filename) {
    vector<string> records;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return records;
    }
    
    string line;
    while (getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \\t\\r\\n"));
        line.erase(line.find_last_not_of(" \\t\\r\\n") + 1);
        
        if (!line.empty()) {
            records.push_back(line);
        }
    }
    
    file.close();
    sort(records.begin(), records.end());
    return records;
}

int main() {
    // Test case 1: Basic key-value pairs
    {
        ofstream file("test1.txt");
        file << "name=John\\n";
        file << "age=30\\n";
        file << "city=Boston\\n";
        file.close();
        
        cout << "Test 1:" << endl;
        vector<string> result = readAndSortFile("test1.txt");
        for (const auto& record : result) {
            cout << record << endl;
        }
    }
    
    // Test case 2: Numeric keys
    {
        ofstream file("test2.txt");
        file << "3=apple\\n";
        file << "1=banana\\n";
        file << "2=cherry\\n";
        file.close();
        
        cout << "\\nTest 2:" << endl;
        vector<string> result = readAndSortFile("test2.txt");
        for (const auto& record : result) {
            cout << record << endl;
        }
    }
    
    // Test case 3: Mixed case
    {
        ofstream file("test3.txt");
        file << "Zebra=animal\\n";
        file << "apple=fruit\\n";
        file << "Banana=fruit\\n";
        file.close();
        
        cout << "\\nTest 3:" << endl;
        vector<string> result = readAndSortFile("test3.txt");
        for (const auto& record : result) {
            cout << record << endl;
        }
    }
    
    // Test case 4: Empty lines
    {
        ofstream file("test4.txt");
        file << "key1=value1\\n";
        file << "\\n";
        file << "key2=value2\\n";
        file << "   \\n";
        file << "key3=value3\\n";
        file.close();
        
        cout << "\\nTest 4:" << endl;
        vector<string> result = readAndSortFile("test4.txt");
        for (const auto& record : result) {
            cout << record << endl;
        }
    }
    
    // Test case 5: Single record
    {
        ofstream file("test5.txt");
        file << "onlykey=onlyvalue\\n";
        file.close();
        
        cout << "\\nTest 5:" << endl;
        vector<string> result = readAndSortFile("test5.txt");
        for (const auto& record : result) {
            cout << record << endl;
        }
    }
    
    return 0;
}
