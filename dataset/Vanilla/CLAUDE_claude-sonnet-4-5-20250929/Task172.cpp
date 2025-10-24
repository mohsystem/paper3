
#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;

class Task172 {
public:
    static int distinctEchoSubstrings(string text) {
        unordered_set<string> distinctSubstrings;
        int n = text.length();
        
        // Try all possible substring lengths (must be even)
        for (int len = 2; len <= n; len += 2) {
            int halfLen = len / 2;
            
            // Try all possible starting positions
            for (int i = 0; i <= n - len; i++) {
                string first = text.substr(i, halfLen);
                string second = text.substr(i + halfLen, halfLen);
                
                if (first == second) {
                    distinctSubstrings.insert(first + second);
                }
            }
        }
        
        return distinctSubstrings.size();
    }
};

int main() {
    // Test case 1
    string test1 = "abcabcabc";
    cout << "Input: \\"" << test1 << "\\"" << endl;
    cout << "Output: " << Task172::distinctEchoSubstrings(test1) << endl;
    cout << endl;
    
    // Test case 2
    string test2 = "leetcodeleetcode";
    cout << "Input: \\"" << test2 << "\\"" << endl;
    cout << "Output: " << Task172::distinctEchoSubstrings(test2) << endl;
    cout << endl;
    
    // Test case 3
    string test3 = "aaaa";
    cout << "Input: \\"" << test3 << "\\"" << endl;
    cout << "Output: " << Task172::distinctEchoSubstrings(test3) << endl;
    cout << endl;
    
    // Test case 4
    string test4 = "abcdef";
    cout << "Input: \\"" << test4 << "\\"" << endl;
    cout << "Output: " << Task172::distinctEchoSubstrings(test4) << endl;
    cout << endl;
    
    // Test case 5
    string test5 = "aabbccdd";
    cout << "Input: \\"" << test5 << "\\"" << endl;
    cout << "Output: " << Task172::distinctEchoSubstrings(test5) << endl;
    
    return 0;
}
