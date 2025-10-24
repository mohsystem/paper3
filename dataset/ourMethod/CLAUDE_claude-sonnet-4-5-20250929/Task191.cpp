
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

// Security: Input validation to ensure string contains only '(' and ')'
// Returns -1 if invalid input detected
bool validateInput(const std::string& s) {
    // Check string length against constraint (0 <= length <= 30000)
    if (s.length() > 30000) {
        return false;
    }
    
    // Validate each character is either '(' or ')'
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] != '(' && s[i] != ')') {
            return false;
        }
    }
    return true;
}

int longestValidParentheses(const std::string& s) {
    // Security: Validate input before processing
    if (!validateInput(s)) {
        return -1; // Invalid input
    }
    
    // Handle empty string case
    if (s.empty()) {
        return 0;
    }
    
    // Security: Use size_t for array indices to prevent integer overflow
    const size_t n = s.length();
    
    // Security: Bounds check - ensure we don't allocate excessive memory\n    // Maximum reasonable size already validated above\n    std::vector<int> dp(n, 0);\n    int maxLen = 0;\n    \n    // Dynamic programming approach\n    // dp[i] represents the length of longest valid parentheses ending at index i\n    for (size_t i = 1; i < n; ++i) {\n        if (s[i] == ')') {\n            if (s[i - 1] == '(') {\n                // Case: ...()\n                // Security: Check bounds before accessing dp[i-2]\n                dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;\n            } else if (i >= 1 && dp[i - 1] > 0) {\n                // Case: ...))\n                // Security: Calculate match_idx with overflow protection\n                size_t match_idx = i - static_cast<size_t>(dp[i - 1]) - 1;\n                \n                // Security: Bounds check before array access\n                if (match_idx < n && s[match_idx] == '(') {
                    dp[i] = dp[i - 1] + 2;
                    // Security: Additional bounds check for match_idx - 1
                    if (match_idx >= 1) {
                        dp[i] += dp[match_idx - 1];
                    }
                }
            }
            // Security: Check for integer overflow before comparison
            if (dp[i] > maxLen) {
                maxLen = dp[i];
            }
        }
    }
    
    return maxLen;
}

int main() {
    // Test case 1
    std::string test1 = "(()";
    std::cout << "Input: \\"" << test1 << "\\" -> Output: " << longestValidParentheses(test1) << std::endl;
    
    // Test case 2
    std::string test2 = ")()())";
    std::cout << "Input: \\"" << test2 << "\\" -> Output: " << longestValidParentheses(test2) << std::endl;
    
    // Test case 3
    std::string test3 = "";
    std::cout << "Input: \\"" << test3 << "\\" -> Output: " << longestValidParentheses(test3) << std::endl;
    
    // Test case 4
    std::string test4 = "((()))";
    std::cout << "Input: \\"" << test4 << "\\" -> Output: " << longestValidParentheses(test4) << std::endl;
    
    // Test case 5
    std::string test5 = "()(())";
    std::cout << "Input: \\"" << test5 << "\\" -> Output: " << longestValidParentheses(test5) << std::endl;
    
    return 0;
}
