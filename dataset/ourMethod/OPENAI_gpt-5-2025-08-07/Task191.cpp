#include <iostream>
#include <string>
#include <vector>
#include <limits>

int longestValidParentheses(const std::string& s) {
    if (s.size() > 30000) {
        return -1;
    }
    for (char c : s) {
        if (c != '(' && c != ')') {
            return -1;
        }
    }
    std::vector<int> st;
    st.reserve(s.size() + 1);
    st.push_back(-1);
    int maxLen = 0;
    for (int i = 0; i < static_cast<int>(s.size()); ++i) {
        if (s[i] == '(') {
            st.push_back(i);
        } else {
            if (!st.empty()) {
                st.pop_back();
            }
            if (st.empty()) {
                st.push_back(i);
            } else {
                int len = i - st.back();
                if (len > maxLen) {
                    maxLen = len;
                }
            }
        }
    }
    return maxLen;
}

int main() {
    std::string tests[5] = {
        "(()",
        ")()())",
        "",
        "()(())",
        "abc"
    };
    for (const auto& t : tests) {
        std::cout << longestValidParentheses(t) << std::endl;
    }
    return 0;
}