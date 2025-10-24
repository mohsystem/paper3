
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

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
        // Input validation
        if (s.empty()) {
            return "";
        }
        
        if (s.length() > 50000) {
            throw invalid_argument("String length exceeds maximum allowed");
        }
        
        // Validate that string contains only lowercase letters
        for (char c : s) {
            if (c < 'a' || c > 'z') {
                throw invalid_argument("String must contain only lowercase letters");
            }
        }
        
        // Use KMP algorithm to find longest palindromic prefix
        string rev = s;
        reverse(rev.begin(), rev.end());
        string combined = s + "#" + rev;
        vector<int> lps = computeLPS(combined);
        
        // Find the part to prepend
        int palindromeLength = lps[combined.length() - 1];
        string toPrepend = rev.substr(0, s.length() - palindromeLength);
        
        return toPrepend + s;
    }
};

int main() {
    // Test cases
    cout << Task174::shortestPalindrome("aacecaaa") << endl;  // "aaacecaaa"
    cout << Task174::shortestPalindrome("abcd") << endl;      // "dcbabcd"
    cout << Task174::shortestPalindrome("") << endl;          // ""
    cout << Task174::shortestPalindrome("a") << endl;         // "a"
    cout << Task174::shortestPalindrome("abbacd") << endl;    // "dcabbacd"
    
    return 0;
}
