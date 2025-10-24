
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

vector<string> parseLine(const string& line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                field += '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    
    return fields;
}

vector<vector<string>> parseCSV(const string& csvContent) {
    vector<vector<string>> records;
    
    if (csvContent.empty()) {
        return records;
    }
    
    stringstream ss(csvContent);
    string line;
    
    while (getline(ss, line)) {
        if (!line.empty()) {
            vector<string> record = parseLine(line);
            records.push_back(record);
        }
    }
    
    return records;
}

void printCSV(const vector<vector<string>>& records) {
    for (const auto& record : records) {
        cout << "[";
        for (size_t i = 0; i < record.size(); i++) {
            cout << "\\"" << record[i] << "\\"";
            if (i < record.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
}

int main() {
    // Test Case 1: Simple CSV
    string csv1 = "Name,Age,City\\nJohn,30,New York\\nJane,25,Los Angeles";
    cout << "Test Case 1 - Simple CSV:" << endl;
    vector<vector<string>> result1 = parseCSV(csv1);
    printCSV(result1);
    cout << endl;
    
    // Test Case 2: CSV with quoted fields
    string csv2 = "Name,Description\\n\\"John Doe\\",\\"Software Engineer\\"\\n\\"Jane Smith\\",\\"Data Scientist\\"";
    cout << "Test Case 2 - CSV with quoted fields:" << endl;
    vector<vector<string>> result2 = parseCSV(csv2);
    printCSV(result2);
    cout << endl;
    
    // Test Case 3: CSV with commas in quoted fields
    string csv3 = "Product,Price,Description\\nLaptop,1200,\\"High performance, 16GB RAM\\"\\nPhone,800,\\"5G enabled, dual camera\\"";
    cout << "Test Case 3 - CSV with commas in quoted fields:" << endl;
    vector<vector<string>> result3 = parseCSV(csv3);
    printCSV(result3);
    cout << endl;
    
    // Test Case 4: CSV with escaped quotes
    string csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She said \\"\\"Goodbye\\"\\"\\"";
    cout << "Test Case 4 - CSV with escaped quotes:" << endl;
    vector<vector<string>> result4 = parseCSV(csv4);
    printCSV(result4);
    cout << endl;
    
    // Test Case 5: Empty and single line CSV
    string csv5 = "Header1,Header2,Header3";
    cout << "Test Case 5 - Single line CSV:" << endl;
    vector<vector<string>> result5 = parseCSV(csv5);
    printCSV(result5);
    
    return 0;
}
