#include <iostream>
#include <string>
#include <stack>
#include <algorithm>
#include <vector>

/**
 * @brief Given a string containing just the characters '(' and ')', returns the length
 * of the longest valid (well-formed) parentheses substring.
 * 
 * @param s The input string consisting of '(' and ')'.
 * @return The length of the longest valid parentheses substring.
 */
int longestValidParentheses(const std::string& s) {
    if (s.length() < 2) {
        return 0;
    }

    int maxLen = 0;
    std::stack<int> st;
    // Push -1 as a sentinel value. It acts as a boundary for the first valid substring.
    st.push(-1); 

    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '(') {
            st.push(i);
        } else { // s[i] == ')'
            st.pop();
            if (st.empty()) {
                // Current ')' is unmatched. Push its index as the new boundary.
                st.push(i);
            } else {
                // A valid pair is found. The length is the current index minus the
                // index of the character just before the start of this valid substring.
                maxLen = std::max(maxLen, i - st.top());
            }
        }
    }
    return maxLen;
}

int main() {
    // Test cases
    std::vector<std::pair<std::string, int>> testCases = {
        {"(()", 2},
        {")()())", 4},
        {"", 0},
        {"()(()", 2},
        {"()(())", 6}
    };

    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase.first << "\", Output: " << longestValidParentheses(testCase.first) << std::endl;
    }

    return 0;
}