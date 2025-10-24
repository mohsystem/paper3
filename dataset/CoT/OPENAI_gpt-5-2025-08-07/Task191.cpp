#include <iostream>
#include <vector>
#include <string>

int longestValidParentheses(const std::string& s) {
    if (s.empty()) return 0;
    std::vector<int> st;
    st.reserve(s.size() + 1);
    st.push_back(-1);
    int maxLen = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '(') {
            st.push_back(static_cast<int>(i));
        } else if (c == ')') {
            if (!st.empty()) st.pop_back();
            if (st.empty()) {
                st.push_back(static_cast<int>(i));
            } else {
                int len = static_cast<int>(i) - st.back();
                if (len > maxLen) maxLen = len;
            }
        }
    }
    return maxLen;
}

int main() {
    std::string tests[] = {
        "(()",
        ")()())",
        "",
        "()(())",
        "((((("
    };
    for (const auto& t : tests) {
        std::cout << longestValidParentheses(t) << std::endl;
    }
    return 0;
}