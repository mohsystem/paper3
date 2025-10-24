
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

vector<string> transpose(const string& filename) {
    vector<string> result;
    vector<vector<string>> matrix;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return result;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> row;
        stringstream ss(line);
        string word;
        
        while (ss >> word) {
            row.push_back(word);
        }
        
        if (!row.empty()) {
            matrix.push_back(row);
        }
    }
    file.close();
    
    if (matrix.empty()) return result;
    
    size_t cols = matrix[0].size();
    for (size_t j = 0; j < cols; j++) {
        string transposedRow;
        for (size_t i = 0; i < matrix.size(); i++) {
            if (j < matrix[i].size()) {
                if (!transposedRow.empty()) {
                    transposedRow += " ";
                }
                transposedRow += matrix[i][j];
            }
        }
        result.push_back(transposedRow);
    }
    
    return result;
}

int main() {
    // Test case 1: Basic example
    {
        ofstream out("test1.txt");
        out << "name age\\n";
        out << "alice 21\\n";
        out << "ryan 30\\n";
        out.close();
        
        cout << "Test 1:" << endl;
        vector<string> result = transpose("test1.txt");
        for (const auto& line : result) {
            cout << line << endl;
        }
        cout << endl;
    }
    
    // Test case 2: Single row
    {
        ofstream out("test2.txt");
        out << "a b c d\\n";
        out.close();
        
        cout << "Test 2:" << endl;
        vector<string> result = transpose("test2.txt");
        for (const auto& line : result) {
            cout << line << endl;
        }
        cout << endl;
    }
    
    // Test case 3: Single column
    {
        ofstream out("test3.txt");
        out << "a\\n";
        out << "b\\n";
        out << "c\\n";
        out.close();
        
        cout << "Test 3:" << endl;
        vector<string> result = transpose("test3.txt");
        for (const auto& line : result) {
            cout << line << endl;
        }
        cout << endl;
    }
    
    // Test case 4: 3x3 matrix
    {
        ofstream out("test4.txt");
        out << "1 2 3\\n";
        out << "4 5 6\\n";
        out << "7 8 9\\n";
        out.close();
        
        cout << "Test 4:" << endl;
        vector<string> result = transpose("test4.txt");
        for (const auto& line : result) {
            cout << line << endl;
        }
        cout << endl;
    }
    
    // Test case 5: Empty file
    {
        ofstream out("test5.txt");
        out.close();
        
        cout << "Test 5:" << endl;
        vector<string> result = transpose("test5.txt");
        if (result.empty()) {
            cout << "Empty result" << endl;
        }
        cout << endl;
    }
    
    return 0;
}
