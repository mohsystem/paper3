
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
        
        string rev = s;
        reverse(rev.begin(), rev.end());
        string combined = s + "#" + rev;
        
        vector<int> lps = computeLPS(combined);
        int longestPalindromePrefix = lps[combined.length() - 1];
        
        string suffix = s.substr(longestPalindromePrefix);
        reverse(suffix.begin(), suffix.end());
        return suffix + s;
    }
};

int main() {
    // Test case 1
    cout << "Test 1: " << Task174::shortestPalindrome("aacecaaa") << endl;
    
    // Test case 2
    cout << "Test 2: " << Task174::shortestPalindrome("abcd") << endl;
    
    // Test case 3
    cout << "Test 3: " << Task174::shortestPalindrome("") << endl;
    
    // Test case 4
    cout << "Test 4: " << Task174::shortestPalindrome("a") << endl;
    
    // Test case 5
    cout << "Test 5: " << Task174::shortestPalindrome("aba") << endl;
    
    return 0;
}
