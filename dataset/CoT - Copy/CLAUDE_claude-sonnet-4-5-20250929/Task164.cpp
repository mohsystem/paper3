
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class Task164 {
public:
    static string extractLastName(const string& fullName) {
        if (fullName.empty()) {
            return "";
        }
        
        // Trim and find last space
        size_t start = fullName.find_first_not_of(" \\t\\n\\r");
        size_t end = fullName.find_last_not_of(" \\t\\n\\r");
        
        if (start == string::npos) {
            return "";
        }
        
        string trimmed = fullName.substr(start, end - start + 1);
        size_t lastSpace = trimmed.rfind(' ');
        
        if (lastSpace == string::npos) {
            return trimmed;
        }
        
        return trimmed.substr(lastSpace + 1);
    }
    
    static vector<string> lastNameLensort(vector<string> names) {
        if (names.empty()) {
            return vector<string>();
        }
        
        sort(names.begin(), names.end(), [](const string& name1, const string& name2) {
            string lastName1 = extractLastName(name1);
            string lastName2 = extractLastName(name2);
            
            // Compare by length first
            if (lastName1.length() != lastName2.length()) {
                return lastName1.length() < lastName2.length();
            }
            
            // If lengths are equal, compare alphabetically
            return lastName1 < lastName2;
        });
        
        return names;
    }
};

int main() {
    // Test case 1
    vector<string> test1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    vector<string> result1 = Task164::lastNameLensort(test1);
    cout << "Test 1: ";
    for (const auto& name : result1) {
        cout << name << ", ";
    }
    cout << endl;
    
    // Test case 2
    vector<string> test2 = {"John Doe", "Jane Smith", "Bob Lee"};
    vector<string> result2 = Task164::lastNameLensort(test2);
    cout << "Test 2: ";
    for (const auto& name : result2) {
        cout << name << ", ";
    }
    cout << endl;
    
    // Test case 3
    vector<string> test3 = {"Alice Brown", "Charlie Brown", "David Green"};
    vector<string> result3 = Task164::lastNameLensort(test3);
    cout << "Test 3: ";
    for (const auto& name : result3) {
        cout << name << ", ";
    }
    cout << endl;
    
    // Test case 4 - Empty vector
    vector<string> test4 = {};
    vector<string> result4 = Task164::lastNameLensort(test4);
    cout << "Test 4: (empty)" << endl;
    
    // Test case 5 - Single name
    vector<string> test5 = {"Michael Jackson"};
    vector<string> result5 = Task164::lastNameLensort(test5);
    cout << "Test 5: ";
    for (const auto& name : result5) {
        cout << name << ", ";
    }
    cout << endl;
    
    return 0;
}
