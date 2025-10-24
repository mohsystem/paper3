
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

bool isIsogram(string str) {
    if (str.empty()) {
        return true;
    }
    
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    
    for (int i = 0; i < str.length(); i++) {
        for (int j = i + 1; j < str.length(); j++) {
            if (str[i] == str[j]) {
                return false;
            }
        }
    }
    
    return true;
}

int main() {
    // Test cases
    cout << boolalpha;
    cout << isIsogram("Dermatoglyphics") << endl;  // true
    cout << isIsogram("aba") << endl;              // false
    cout << isIsogram("moOse") << endl;            // false
    cout << isIsogram("") << endl;                 // true
    cout << isIsogram("isogram") << endl;          // false
    
    return 0;
}
