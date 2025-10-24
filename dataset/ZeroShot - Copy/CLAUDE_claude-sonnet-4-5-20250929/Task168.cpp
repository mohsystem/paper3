
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string tweakLetters(string str, vector<int> tweaks) {
    if (str.length() != tweaks.size()) {
        return str;
    }
    
    string result = "";
    
    for (size_t i = 0; i < str.length(); i++) {
        char currentChar = str[i];
        int tweak = tweaks[i];
        
        // Apply the tweak to the character
        char newChar = currentChar + tweak;
        
        // Handle wrapping for lowercase letters
        if (currentChar >= 'a' && currentChar <= 'z') {
            if (newChar > 'z') {
                newChar = 'a' + (newChar - 'z' - 1);
            } else if (newChar < 'a') {
                newChar = 'z' - ('a' - newChar - 1);
            }
        }
        
        result += newChar;
    }
    
    return result;
}

int main() {
    // Test case 1
    cout << tweakLetters("apple", {0, 1, -1, 0, -1}) << endl;
    // Expected: "aqold"
    
    // Test case 2
    cout << tweakLetters("many", {0, 0, 0, -1}) << endl;
    // Expected: "manx"
    
    // Test case 3
    cout << tweakLetters("rhino", {1, 1, 1, 1, 1}) << endl;
    // Expected: "sijop"
    
    // Test case 4
    cout << tweakLetters("zebra", {1, 0, 0, 0, -1}) << endl;
    // Expected: "zebrq"
    
    // Test case 5
    cout << tweakLetters("hello", {0, 0, 0, 0, 0}) << endl;
    // Expected: "hello"
    
    return 0;
}
