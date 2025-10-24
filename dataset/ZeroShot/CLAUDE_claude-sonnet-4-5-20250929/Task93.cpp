
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

using namespace std;

class Task93 {
public:
    static vector<pair<string, string>> readAndSortFile(const string& filename) {
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
            
            if (start == string::npos) continue;
            
            line = line.substr(start, end - start + 1);
            
            size_t pos = line.find('=');
            if (pos == string::npos) continue;
            
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            
            // Trim key and value
            key.erase(key.find_last_not_of(" \\t") + 1);
            value.erase(0, value.find_first_not_of(" \\t"));
            
            records.push_back(make_pair(key, value));
        }
        
        file.close();
        
        // Sort by key
        sort(records.begin(), records.end(), 
             [](const pair<string, string>& a, const pair<string, string>& b) {
                 return a.first < b.first;
             });
        
        return records;
    }
};

int main() {
    // Test case 1
    cout << "Test Case 1:" << endl;
    ofstream out1("test1.txt");
    out1 << "name=John\\n";
    out1 << "age=30\\n";
    out1 << "city=NewYork\\n";
    out1.close();
    
    vector<pair<string, string>> result1 = Task93::readAndSortFile("test1.txt");
    for (const auto& entry : result1) {
        cout << entry.first << "=" << entry.second << endl;
    }
    
    // Test case 2
    cout << "\\nTest Case 2:" << endl;
    ofstream out2("test2.txt");
    out2 << "zebra=animal\\n";
    out2 << "apple=fruit\\n";
    out2 << "car=vehicle\\n";
    out2.close();
    
    vector<pair<string, string>> result2 = Task93::readAndSortFile("test2.txt");
    for (const auto& entry : result2) {
        cout << entry.first << "=" << entry.second << endl;
    }
    
    // Test case 3
    cout << "\\nTest Case 3:" << endl;
    ofstream out3("test3.txt");
    out3 << "key3=value3\\n";
    out3 << "key1=value1\\n";
    out3 << "key2=value2\\n";
    out3.close();
    
    vector<pair<string, string>> result3 = Task93::readAndSortFile("test3.txt");
    for (const auto& entry : result3) {
        cout << entry.first << "=" << entry.second << endl;
    }
    
    // Test case 4 - Empty file
    cout << "\\nTest Case 4:" << endl;
    ofstream out4("test4.txt");
    out4.close();
    
    vector<pair<string, string>> result4 = Task93::readAndSortFile("test4.txt");
    cout << "Records found: " << result4.size() << endl;
    
    // Test case 5
    cout << "\\nTest Case 5:" << endl;
    ofstream out5("test5.txt");
    out5 << "user.name=admin\\n";
    out5 << "server.port=8080\\n";
    out5 << "app.version=1.0\\n";
    out5.close();
    
    vector<pair<string, string>> result5 = Task93::readAndSortFile("test5.txt");
    for (const auto& entry : result5) {
        cout << entry.first << "=" << entry.second << endl;
    }
    
    return 0;
}
