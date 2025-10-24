
#include <iostream>
#include <string>
#include <set>
#include <algorithm>

using namespace std;

string longest(const string& s1, const string& s2) {
    set<char> uniqueChars;
    
    for (char c : s1) {
        if (c >= 'a' && c <= 'z') {
            uniqueChars.insert(c);
        }
    }
    
    for (char c : s2) {
        if (c >= 'a' && c <= 'z') {
            uniqueChars.insert(c);
        }
    }
    
    string result;
    for (char c : uniqueChars) {
        result += c;
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
    string b2 = "abcdefghijklmnopqrstuvwxyz";
    cout << "Test 2: " << longest(a2, b2) << endl;
    
    // Test case 3
    string a3 = "aretheyhere";
    string b3 = "yestheyarehere";
    cout << "Test 3: " << longest(a3, b3) << endl;
    
    // Test case 4
    string a4 = "loopingisfunbutdangerous";
    string b4 = "lessdangerousthancoding";
    cout << "Test 4: " << longest(a4, b4) << endl;
    
    // Test case 5
    string a5 = "";
    string b5 = "abc";
    cout << "Test 5: " << longest(a5, b5) << endl;
    
    return 0;
}
