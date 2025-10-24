
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Task191 {
public:
    static int longestValidParentheses(string s) {
        // Input validation
        if (s.empty() || s.length() < 2) {
            return 0;
        }
        
        int maxLen = 0;
        vector<int> dp(s.length(), 0);
        
        for (int i = 1; i < s.length(); i++) {
            if (s[i] == ')') {
                if (s[i - 1] == '(') {
                    // Case: ...()
                    dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
                } else if (i - dp[i - 1] > 0 && s[i - dp[i - 1] - 1] == '(') {
                    // Case: ...))
                    dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
                }
                maxLen = max(maxLen, dp[i]);
            }
        }
        
        return maxLen;
    }
};

int main() {
    // Test cases
    cout << "Test 1: " << Task191::longestValidParentheses("(()") << " (Expected: 2)" << endl;
    cout << "Test 2: " << Task191::longestValidParentheses(")()())") << " (Expected: 4)" << endl;
    cout << "Test 3: " << Task191::longestValidParentheses("") << " (Expected: 0)" << endl;
    cout << "Test 4: " << Task191::longestValidParentheses("()(()") << " (Expected: 2)" << endl;
    cout << "Test 5: " << Task191::longestValidParentheses("(()())") << " (Expected: 6)" << endl;
    
    return 0;
}
