
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Task174 {
public:
    static vector<int> computeLPS(const string& s) {
        int n = s.length();
        vector<int> lps(n, 0);
        int len = 0;
        int i = 1;
        
        while (i < n) {
            if (s[i] == s[len]) {
                len++;
                lps[i] = len;
                i++;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    i++;
                }
            }
        }
        
        return lps;
    }
    
    static string shortestPalindrome(const string& s) {
        if (s.empty()) {
            return s;
        }
        
        // Find the longest palindrome prefix using KMP algorithm
        string rev = s;
        reverse(rev.begin(), rev.end());
        string combined = s + "#" + rev;
        
        vector<int> lps = computeLPS(combined);
        int longestPalindromePrefix = lps[combined.length() - 1];
        
        // Add the reversed suffix to the front
        string suffix = s.substr(longestPalindromePrefix);
        string prefixToAdd = suffix;
        reverse(prefixToAdd.begin(), prefixToAdd.end());
        
        return prefixToAdd + s;
    }
};

int main() {
    // Test case 1
    string test1 = "aacecaaa";
    cout << "Input: " << test1 << endl;
    cout << "Output: " << Task174::shortestPalindrome(test1) << endl;
    cout << endl;
    
    // Test case 2
    string test2 = "abcd";
    cout << "Input: " << test2 << endl;
    cout << "Output: " << Task174::shortestPalindrome(test2) << endl;
    cout << endl;
    
    // Test case 3
    string test3 = "";
    cout << "Input: \\"" << test3 << "\\"" << endl;
    cout << "Output: \\"" << Task174::shortestPalindrome(test3) << "\\"" << endl;
    cout << endl;
    
    // Test case 4
    string test4 = "a";
    cout << "Input: " << test4 << endl;
    cout << "Output: " << Task174::shortestPalindrome(test4) << endl;
    cout << endl;
    
    // Test case 5
    string test5 = "abbacd";
    cout << "Input: " << test5 << endl;
    cout << "Output: " << Task174::shortestPalindrome(test5) << endl;
    cout << endl;
    
    return 0;
}
