
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
        cerr << "Error: Could not open file '" << filename << "'" << endl;
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
    // Test case 1: File with simple key-value pairs
    {
        ofstream file("test1.txt");
        file << "name=John\\n";
        file << "age=30\\n";
        file << "city=New York\\n";
        file.close();
        
        cout << "Test 1:" << endl;
        vector<string> result1 = readAndSortFile("test1.txt");
        for (const string& record : result1) {
            cout << record << endl;
        }
        cout << endl;
    }
    
    // Test case 2: File with numeric keys
    {
        ofstream file("test2.txt");
        file << "3=three\\n";
        file << "1=one\\n";
        file << "2=two\\n";
        file.close();
        
        cout << "Test 2:" << endl;
        vector<string> result2 = readAndSortFile("test2.txt");
        for (const string& record : result2) {
            cout << record << endl;
        }
        cout << endl;
    }
    
    // Test case 3: File with mixed keys
    {
        ofstream file("test3.txt");
        file << "zebra=animal\\n";
        file << "apple=fruit\\n";
        file << "car=vehicle\\n";
        file << "banana=fruit\\n";
        file.close();
        
        cout << "Test 3:" << endl;
        vector<string> result3 = readAndSortFile("test3.txt");
        for (const string& record : result3) {
            cout << record << endl;
        }
        cout << endl;
    }
    
    // Test case 4: Empty file
    {
        ofstream file("test4.txt");
        file.close();
        
        cout << "Test 4 (empty file):" << endl;
        vector<string> result4 = readAndSortFile("test4.txt");
        cout << "Records count: " << result4.size() << endl;
        cout << endl;
    }
    
    // Test case 5: File with duplicate keys
    {
        ofstream file("test5.txt");
        file << "key1=value1\\n";
        file << "key3=value3\\n";
        file << "key1=value2\\n";
        file << "key2=value4\\n";
        file.close();
        
        cout << "Test 5:" << endl;
        vector<string> result5 = readAndSortFile("test5.txt");
        for (const string& record : result5) {
            cout << record << endl;
        }
        cout << endl;
    }
    
    return 0;
}
