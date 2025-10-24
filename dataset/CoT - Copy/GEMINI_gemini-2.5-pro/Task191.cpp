#include <iostream>
#include <string>
#include <stack>
#include <algorithm>

class Task191 {
public:
    /**
     * @brief Given a string containing just the characters '(' and ')', returns the length of the longest 
     * valid (well-formed) parentheses substring.
     *
     * This method uses a stack to keep track of the indices of the parentheses.
     * The stack stores the indices of '(' characters. An initial -1 is pushed to the stack
     * to act as a base for calculating the length of a valid substring.
     *
     * When a '(' is encountered, its index is pushed onto the stack.
     * When a ')' is encountered, the top of the stack is popped.
     * - If the stack becomes empty after popping, it means the current ')' does not have a matching '('.
     *   So, the index of this ')' is pushed to serve as the new base for future valid substrings.
     * - If the stack is not empty after popping, a valid pair is formed. The length of the
     *   current valid substring is the difference between the current index and the index at the top of the stack.
     *   The maximum length is updated accordingly.
     * 
     * @param s The input string containing only '(' and ')' characters.
     * @return The length of the longest valid parentheses substring.
     */
    int longestValidParentheses(const std::string& s) {
        int maxLength = 0;
        std::stack<int> st;
        st.push(-1); // Base for the first valid substring

        for (int i = 0; i < s.length(); ++i) {
            if (s[i] == '(') {
                st.push(i);
            } else { // s[i] == ')'
                st.pop();
                if (st.empty()) {
                    // This ')' is unmatched, it becomes the new base
                    st.push(i);
                } else {
                    // A match is found. Calculate the length.
                    maxLength = std::max(maxLength, i - st.top());
                }
            }
        }
        return maxLength;
    }
};

int main() {
    Task191 solution;
    
    // Test Case 1
    std::string s1 = "(()";
    std::cout << "Input: \"" << s1 << "\", Output: " << solution.longestValidParentheses(s1) << std::endl;

    // Test Case 2
    std::string s2 = ")()())";
    std::cout << "Input: \"" << s2 << "\", Output: " << solution.longestValidParentheses(s2) << std::endl;

    // Test Case 3
    std::string s3 = "";
    std::cout << "Input: \"" << s3 << "\", Output: " << solution.longestValidParentheses(s3) << std::endl;

    // Test Case 4
    std::string s4 = "()(()";
    std::cout << "Input: \"" << s4 << "\", Output: " << solution.longestValidParentheses(s4) << std::endl;

    // Test Case 5
    std::string s5 = "((()))";
    std::cout << "Input: \"" << s5 << "\", Output: " << solution.longestValidParentheses(s5) << std::endl;

    return 0;
}