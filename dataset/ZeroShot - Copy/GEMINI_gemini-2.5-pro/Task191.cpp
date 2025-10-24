#include <iostream>
#include <string>
#include <stack>
#include <algorithm>
#include <vector>

/**
 * @brief Given a string containing just the characters '(' and ')', returns the length of the
 * longest valid (well-formed) parentheses substring.
 * 
 * @param s The input string consisting of '(' and ')'.
 * @return The length of the longest valid parentheses substring.
 */
int longestValidParentheses(const std::string& s) {
    if (s.length() < 2) {
        return 0;
    }

    int maxLength = 0;
    // The stack will store indices of '(' characters.
    std::stack<int> st;
    // Push -1 as a sentinel value to mark the base for a valid substring.
    st.push(-1);

    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '(') {
            st.push(i);
        } else { // s[i] == ')'
            st.pop();
            if (st.empty()) {
                // This ')' does not have a matching '('.
                // Push its index to serve as the new base.
                st.push(i);
            } else {
                // A valid pair is found.
                // The length is the current index minus the index of the element
                // at the top of the stack (which is the new base).
                maxLength = std::max(maxLength, i - st.top());
            }
        }
    }
    return maxLength;
}

void runTestCase(const std::string& s, int expected) {
    int result = longestValidParentheses(s);
    std::cout << "Test Case: \"" << s << "\"" << std::endl;
    std::cout << "Result: " << result << std::endl;
    std::cout << "Expected: " << expected << std::endl;
    std::cout << (result == expected ? "Passed" : "Failed") << std::endl;
    std::cout << std::endl;
}

int main() {
    runTestCase("(()", 2);
    runTestCase(")()())", 4);
    runTestCase("", 0);
    runTestCase("()(()", 2);
    runTestCase("((()))())", 8);
    return 0;
}