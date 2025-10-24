
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

vector<string> transpose(const string& filename) {
    vector<string> result;
    ifstream file(filename);
    vector<vector<string>> rows;
    string line;
    int maxCols = 0;
    
    while (getline(file, line)) {
        vector<string> cols;
        stringstream ss(line);
        string col;
        while (ss >> col) {
            cols.push_back(col);
        }
        rows.push_back(cols);
        maxCols = max(maxCols, (int)cols.size());
    }
    file.close();
    
    for (int col = 0; col < maxCols; col++) {
        string transposed = "";
        for (int row = 0; row < rows.size(); row++) {
            if (col < rows[row].size()) {
                if (!transposed.empty()) transposed += " ";
                transposed += rows[row][col];
            }
        }
        result.push_back(transposed);
    }
    
    return result;
}

int main() {
    // Test case 1
    ofstream out1("test1.txt");
    out1 << "name age\\n";
    out1 << "alice 21\\n";
    out1 << "ryan 30\\n";
    out1.close();
    cout << "Test 1:" << endl;
    for (const auto& line : transpose("test1.txt")) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 2
    ofstream out2("test2.txt");
    out2 << "a b c\\n";
    out2 << "d e f\\n";
    out2 << "g h i\\n";
    out2.close();
    cout << "Test 2:" << endl;
    for (const auto& line : transpose("test2.txt")) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 3
    ofstream out3("test3.txt");
    out3 << "1 2\\n";
    out3 << "3 4\\n";
    out3.close();
    cout << "Test 3:" << endl;
    for (const auto& line : transpose("test3.txt")) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 4
    ofstream out4("test4.txt");
    out4 << "x\\n";
    out4 << "y\\n";
    out4 << "z\\n";
    out4.close();
    cout << "Test 4:" << endl;
    for (const auto& line : transpose("test4.txt")) {
        cout << line << endl;
    }
    cout << endl;
    
    // Test case 5
    ofstream out5("test5.txt");
    out5 << "first second third fourth\\n";
    out5 << "1 2 3 4\\n";
    out5.close();
    cout << "Test 5:" << endl;
    for (const auto& line : transpose("test5.txt")) {
        cout << line << endl;
    }
    cout << endl;
    
    return 0;
}
