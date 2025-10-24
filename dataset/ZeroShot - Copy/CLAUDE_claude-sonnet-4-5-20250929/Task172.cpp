
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;

int distinctEchoSubstrings(string text) {
    unordered_set<string> distinctSubstrings;
    int n = text.length();
    
    // Try all possible substring lengths (must be even)
    for (int len = 2; len <= n; len += 2) {
        int halfLen = len / 2;
        
        // Try all starting positions
        for (int i = 0; i <= n - len; i++) {
            string firstHalf = text.substr(i, halfLen);
            string secondHalf = text.substr(i + halfLen, halfLen);
            
            if (firstHalf == secondHalf) {
                distinctSubstrings.insert(text.substr(i, len));
            }
        }
    }
    
    return distinctSubstrings.size();
}

int main() {
    // Test case 1
    cout << "Test 1: " << distinctEchoSubstrings("abcabcabc") << endl; // Expected: 3
    
    // Test case 2
    cout << "Test 2: " << distinctEchoSubstrings("leetcodeleetcode") << endl; // Expected: 2
    
    // Test case 3
    cout << "Test 3: " << distinctEchoSubstrings("aa") << endl; // Expected: 1
    
    // Test case 4
    cout << "Test 4: " << distinctEchoSubstrings("aaaa") << endl; // Expected: 2
    
    // Test case 5
    cout << "Test 5: " << distinctEchoSubstrings("abcdefg") << endl; // Expected: 0
    
    return 0;
}
