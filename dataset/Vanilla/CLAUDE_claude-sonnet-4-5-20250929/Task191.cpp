
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Task191 {
public:
    static int longestValidParentheses(string s) {
        if (s.empty()) {
            return 0;
        }
        
        int maxLen = 0;
        vector<int> dp(s.length(), 0);
        
        for (int i = 1; i < s.length(); i++) {
            if (s[i] == ')') {
                if (s[i - 1] == '(') {
                    dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
                } else if (i - dp[i - 1] > 0 && s[i - dp[i - 1] - 1] == '(') {
                    dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
                }
                maxLen = max(maxLen, dp[i]);
            }
        }
        
        return maxLen;
    }
};

int main() {
    // Test case 1
    cout << "Test 1: " << Task191::longestValidParentheses("(()") << endl;
    
    // Test case 2
    cout << "Test 2: " << Task191::longestValidParentheses(")()())") << endl;
    
    // Test case 3
    cout << "Test 3: " << Task191::longestValidParentheses("") << endl;
    
    // Test case 4
    cout << "Test 4: " << Task191::longestValidParentheses("()(()") << endl;
    
    // Test case 5
    cout << "Test 5: " << Task191::longestValidParentheses("(()())") << endl;
    
    return 0;
}
