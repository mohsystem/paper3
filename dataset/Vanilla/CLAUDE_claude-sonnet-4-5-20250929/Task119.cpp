
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class Task119 {
public:
    static vector<vector<string>> parseCSV(const string& csvContent) {
        vector<vector<string>> records;
        if (csvContent.empty()) {
            return records;
        }
        
        stringstream ss(csvContent);
        string line;
        
        while (getline(ss, line)) {
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
            records.push_back(fields);
        }
        
        return records;
    }
    
    static void printCSV(const vector<vector<string>>& records) {
        for (const auto& record : records) {
            cout << "[";
            for (size_t i = 0; i < record.size(); i++) {
                cout << "\\"" << record[i] << "\\"";
                if (i < record.size() - 1) cout << ", ";
            }
            cout << "]" << endl;
        }
    }
};

int main() {
    // Test case 1: Simple CSV
    cout << "Test Case 1: Simple CSV" << endl;
    string csv1 = "Name,Age,City\\nJohn,30,New York\\nAlice,25,London";
    vector<vector<string>> result1 = Task119::parseCSV(csv1);
    Task119::printCSV(result1);
    cout << endl;
    
    // Test case 2: CSV with quoted fields
    cout << "Test Case 2: CSV with quoted fields" << endl;
    string csv2 = "Name,Description\\nProduct A,\\"This is a product\\"\\nProduct B,\\"Another product\\"";
    vector<vector<string>> result2 = Task119::parseCSV(csv2);
    Task119::printCSV(result2);
    cout << endl;
    
    // Test case 3: CSV with commas in quoted fields
    cout << "Test Case 3: CSV with commas in quoted fields" << endl;
    string csv3 = "Name,Address\\nJohn,\\"123 Main St, Apt 4\\"\\nAlice,\\"456 Oak Ave, Suite 10\\"";
    vector<vector<string>> result3 = Task119::parseCSV(csv3);
    Task119::printCSV(result3);
    cout << endl;
    
    // Test case 4: CSV with escaped quotes
    cout << "Test Case 4: CSV with escaped quotes" << endl;
    string csv4 = "Title,Quote\\nBook,\\"He said \\"\\"Hello\\"\\"\\"\\nMovie,\\"She replied \\"\\"Hi\\"\\"\\"";
    vector<vector<string>> result4 = Task119::parseCSV(csv4);
    Task119::printCSV(result4);
    cout << endl;
    
    // Test case 5: Empty and mixed content
    cout << "Test Case 5: Empty and mixed content" << endl;
    string csv5 = "A,B,C\\n1,,3\\n,5,\\n7,8,9";
    vector<vector<string>> result5 = Task119::parseCSV(csv5);
    Task119::printCSV(result5);
    
    return 0;
}
