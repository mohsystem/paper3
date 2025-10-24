
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

vector<string> transposeFile(const string& filename) {
    vector<string> result;
    vector<vector<string>> rows;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return result;
    }
    
    string line;
    while (getline(file, line)) {
        vector<string> columns;
        stringstream ss(line);
        string column;
        while (ss >> column) {
            columns.push_back(column);
        }
        rows.push_back(columns);
    }
    file.close();
    
    if (rows.empty()) {
        return result;
    }
    
    int numCols = rows[0].size();
    for (int col = 0; col < numCols; col++) {
        string transposedRow = "";
        for (int row = 0; row < rows.size(); row++) {
            if (row > 0) {
                transposedRow += " ";
            }
            transposedRow += rows[row][col];
        }
        result.push_back(transposedRow);
    }
    
    return result;
}

int main() {
    // Test case 1: Basic example
    ofstream out1("test1.txt");
    out1 << "name age\\n";
    out1 << "alice 21\\n";
    out1 << "ryan 30\\n";
    out1.close();
    
    cout << "Test 1:" << endl;
    vector<string> result1 = transposeFile("test1.txt");
    for (const string& line : result1) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 2: Single row
    ofstream out2("test2.txt");
    out2 << "a b c d\\n";
    out2.close();
    
    cout << "Test 2:" << endl;
    vector<string> result2 = transposeFile("test2.txt");
    for (const string& line : result2) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 3: Single column
    ofstream out3("test3.txt");
    out3 << "a\\n";
    out3 << "b\\n";
    out3 << "c\\n";
    out3.close();
    
    cout << "Test 3:" << endl;
    vector<string> result3 = transposeFile("test3.txt");
    for (const string& line : result3) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 4: 3x3 matrix
    ofstream out4("test4.txt");
    out4 << "1 2 3\\n";
    out4 << "4 5 6\\n";
    out4 << "7 8 9\\n";
    out4.close();
    
    cout << "Test 4:" << endl;
    vector<string> result4 = transposeFile("test4.txt");
    for (const string& line : result4) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 5: Multiple columns
    ofstream out5("test5.txt");
    out5 << "first last age city\\n";
    out5 << "john doe 25 NYC\\n";
    out5 << "jane smith 30 LA\\n";
    out5.close();
    
    cout << "Test 5:" << endl;
    vector<string> result5 = transposeFile("test5.txt");
    for (const string& line : result5) {
        cout << line << endl;
    }
    cout << endl;
    
    return 0;
}
