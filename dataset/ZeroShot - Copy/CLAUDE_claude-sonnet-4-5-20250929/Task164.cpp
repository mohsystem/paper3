
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

string getLastName(const string& fullName) {
    size_t pos = fullName.find_last_of(' ');
    if (pos != string::npos) {
        return fullName.substr(pos + 1);
    }
    return fullName;
}

vector<string> lastNameLensort(vector<string> names) {
    if (names.empty()) {
        return names;
    }
    
    sort(names.begin(), names.end(), [](const string& a, const string& b) {
        string lastNameA = getLastName(a);
        string lastNameB = getLastName(b);
        
        if (lastNameA.length() != lastNameB.length()) {
            return lastNameA.length() < lastNameB.length();
        }
        return lastNameA < lastNameB;
    });
    
    return names;
}

void printVector(const vector<string>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << "\\"" << vec[i] << "\\"";
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
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
    cout << "Test 1: ";
    printVector(lastNameLensort(test1));
    
    // Test case 2
    vector<string> test2 = {
        "John Smith",
        "Jane Doe",
        "Bob Lee"
    };
    cout << "Test 2: ";
    printVector(lastNameLensort(test2));
    
    // Test case 3
    vector<string> test3 = {
        "Alice Johnson",
        "Charlie Brown",
        "David Anderson"
    };
    cout << "Test 3: ";
    printVector(lastNameLensort(test3));
    
    // Test case 4
    vector<string> test4 = {
        "Emily White",
        "Michael Black",
        "Sarah Green"
    };
    cout << "Test 4: ";
    printVector(lastNameLensort(test4));
    
    // Test case 5
    vector<string> test5 = {
        "Tom Martinez",
        "Lisa Garcia",
        "Kevin Rodriguez"
    };
    cout << "Test 5: ";
    printVector(lastNameLensort(test5));
    
    return 0;
}
