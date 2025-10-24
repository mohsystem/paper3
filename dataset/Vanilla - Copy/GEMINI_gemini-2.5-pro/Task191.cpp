#include <iostream>
#include <string>
#include <algorithm>

int longestValidParentheses(std::string s) {
    int left = 0, right = 0, maxLength = 0;
    
    // Left to Right scan
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '(') {
            left++;
        } else {
            right++;
        }
        if (left == right) {
            maxLength = std::max(maxLength, 2 * right);
        } else if (right > left) {
            left = 0;
            right = 0;
        }
    }
    
    left = 0;
    right = 0;
    
    // Right to Left scan
    for (int i = s.length() - 1; i >= 0; --i) {
        if (s[i] == '(') {
            left++;
        } else {
            right++;
        }
        if (left == right) {
            maxLength = std::max(maxLength, 2 * left);
        } else if (left > right) {
            left = 0;
            right = 0;
        }
    }
    
    return maxLength;
}

int main() {
    // Test Case 1
    std::string s1 = "(()";
    std::cout << "Input: " << s1 << ", Output: " << longestValidParentheses(s1) << std::endl;

    // Test Case 2
    std::string s2 = ")()())";
    std::cout << "Input: " << s2 << ", Output: " << longestValidParentheses(s2) << std::endl;

    // Test Case 3
    std::string s3 = "";
    std::cout << "Input: " << s3 << ", Output: " << longestValidParentheses(s3) << std::endl;

    // Test Case 4
    std::string s4 = "()(()";
    std::cout << "Input: " << s4 << ", Output: " << longestValidParentheses(s4) << std::endl;

    // Test Case 5
    std::string s5 = "()(())";
    std::cout << "Input: " << s5 << ", Output: " << longestValidParentheses(s5) << std::endl;
    
    return 0;
}