#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

int longestValidParentheses(const std::string& s) {
    if (s.empty()) return 0;
    std::vector<int> st;
    st.reserve(s.size() + 1);
    st.push_back(-1);
    int maxLen = 0;
    for (int i = 0; i < static_cast<int>(s.size()); ++i) {
        char c = s[i];
        if (c == '(') {
            st.push_back(i);
        } else if (c == ')') {
            if (!st.empty()) st.pop_back();
            if (st.empty()) {
                st.push_back(i);
            } else {
                int len = i - st.back();
                if (len > maxLen) maxLen = len;
            }
        } else {
            // Ignore unexpected characters
        }
    }
    return maxLen;
}

int main() {
    std::vector<std::string> tests = {
        "(()",
        ")()())",
        "",
        "()(())",
        "())((())"
    };
    for (const auto& t : tests) {
        std::cout << "Input: \"" << t << "\" -> " << longestValidParentheses(t) << "\n";
    }
    return 0;
}