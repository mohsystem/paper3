
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

bool isPangram(string sentence) {
    if (sentence.empty()) {
        return false;
    }
    
    bool alphabet[26] = {false};
    
    for (char c : sentence) {
        char lower = tolower(c);
        if (lower >= 'a' && lower <= 'z') {
            alphabet[lower - 'a'] = true;
        }
    }
    
    for (int i = 0; i < 26; i++) {
        if (!alphabet[i]) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test case 1
    string test1 = "The quick brown fox jumps over the lazy dog";
    cout << "Test 1: \\"" << test1 << "\\" -> " << (isPangram(test1) ? "true" : "false") << endl;
    
    // Test case 2
    string test2 = "Hello World";
    cout << "Test 2: \\"" << test2 << "\\" -> " << (isPangram(test2) ? "true" : "false") << endl;
    
    // Test case 3
    string test3 = "abcdefghijklmnopqrstuvwxyz";
    cout << "Test 3: \\"" << test3 << "\\" -> " << (isPangram(test3) ? "true" : "false") << endl;
    
    // Test case 4
    string test4 = "Pack my box with five dozen liquor jugs!";
    cout << "Test 4: \\"" << test4 << "\\" -> " << (isPangram(test4) ? "true" : "false") << endl;
    
    // Test case 5
    string test5 = "1234567890!@#$%^&*()";
    cout << "Test 5: \\"" << test5 << "\\" -> " << (isPangram(test5) ? "true" : "false") << endl;
    
    return 0;
}
