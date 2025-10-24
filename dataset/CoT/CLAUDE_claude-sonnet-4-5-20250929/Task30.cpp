
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

string longest(const string& s1, const string& s2) {
    bool seen[26];
    memset(seen, false, sizeof(seen));
    
    for (size_t i = 0; i < s1.length(); i++) {
        char c = s1[i];
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }
    
    for (size_t i = 0; i < s2.length(); i++) {
        char c = s2[i];
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }
    
    string result = "";
    for (int i = 0; i < 26; i++) {
        if (seen[i]) {
            result += (char)('a' + i);
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    string a1 = "xyaabbbccccdefww";
    string b1 = "xxxxyyyyabklmopq";
    cout << "Test 1: " << longest(a1, b1) << endl;
    
    // Test case 2
    string a2 = "abcdefghijklmnopqrstuvwxyz";
    cout << "Test 2: " << longest(a2, a2) << endl;
    
    // Test case 3
    string a3 = "abc";
    string b3 = "xyz";
    cout << "Test 3: " << longest(a3, b3) << endl;
    
    // Test case 4
    string a4 = "zzz";
    string b4 = "aaa";
    cout << "Test 4: " << longest(a4, b4) << endl;
    
    // Test case 5
    string a5 = "";
    string b5 = "abc";
    cout << "Test 5: " << longest(a5, b5) << endl;
    
    return 0;
}
