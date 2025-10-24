
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

string getLastName(const string& name) {
    size_t pos = name.find_last_of(' ');
    if (pos != string::npos) {
        return name.substr(pos + 1);
    }
    return name;
}

vector<string> lastNameLensort(vector<string> names) {
    sort(names.begin(), names.end(), [](const string& name1, const string& name2) {
        string lastName1 = getLastName(name1);
        string lastName2 = getLastName(name2);
        
        if (lastName1.length() != lastName2.length()) {
            return lastName1.length() < lastName2.length();
        }
        return lastName1 < lastName2;
    });
    
    return names;
}

int main() {
    // Test case 1
    vector<string> test1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    vector<string> result1 = lastNameLensort(test1);
    cout << "Test 1: ";
    for (const auto& name : result1) cout << name << ", ";
    cout << endl;
    
    // Test case 2
    vector<string> test2 = {
        "John Doe",
        "Jane Smith",
        "Bob Lee"
    };
    vector<string> result2 = lastNameLensort(test2);
    cout << "Test 2: ";
    for (const auto& name : result2) cout << name << ", ";
    cout << endl;
    
    // Test case 3
    vector<string> test3 = {
        "Alice Johnson",
        "Charlie Brown",
        "David Green"
    };
    vector<string> result3 = lastNameLensort(test3);
    cout << "Test 3: ";
    for (const auto& name : result3) cout << name << ", ";
    cout << endl;
    
    // Test case 4
    vector<string> test4 = {
        "Emily White",
        "Frank Black",
        "Grace Blue"
    };
    vector<string> result4 = lastNameLensort(test4);
    cout << "Test 4: ";
    for (const auto& name : result4) cout << name << ", ";
    cout << endl;
    
    // Test case 5
    vector<string> test5 = {
        "Michael Anderson",
        "Sarah Williams",
        "Tom Martinez"
    };
    vector<string> result5 = lastNameLensort(test5);
    cout << "Test 5: ";
    for (const auto& name : result5) cout << name << ", ";
    cout << endl;
    
    return 0;
}
