
#include <iostream>
#include <string>
#include <unordered_set>
#include <cctype>

using namespace std;

class Task172 {
public:
    static int distinctEchoSubstrings(const string& text) {
        // Input validation
        if (text.empty() || text.length() > 2000) {
            return 0;
        }
        
        // Validate that text contains only lowercase English letters
        for (char c : text) {
            if (c < 'a' || c > 'z') {
                return 0;
            }
        }
        
        unordered_set<string> distinctSubstrings;
        int n = text.length();
        
        // Iterate through all possible substring lengths (must be even)
        for (int len = 2; len <= n; len += 2) {
            int halfLen = len / 2;
            
            // Iterate through all possible starting positions
            for (int i = 0; i <= n - len; i++) {
                // Extract the two halves
                string firstHalf = text.substr(i, halfLen);
                string secondHalf = text.substr(i + halfLen, halfLen);
                
                // Check if both halves are equal
                if (firstHalf == secondHalf) {
                    distinctSubstrings.insert(firstHalf + secondHalf);
                }
            }
        }
        
        return distinctSubstrings.size();
    }
};

int main() {
    // Test case 1
    cout << "Test 1: " << Task172::distinctEchoSubstrings("abcabcabc") << endl; // Expected: 3
    
    // Test case 2
    cout << "Test 2: " << Task172::distinctEchoSubstrings("leetcodeleetcode") << endl; // Expected: 2
    
    // Test case 3
    cout << "Test 3: " << Task172::distinctEchoSubstrings("aa") << endl; // Expected: 1
    
    // Test case 4
    cout << "Test 4: " << Task172::distinctEchoSubstrings("a") << endl; // Expected: 0
    
    // Test case 5
    cout << "Test 5: " << Task172::distinctEchoSubstrings("aaaa") << endl; // Expected: 2
    
    return 0;
}
